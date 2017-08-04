#pragma once

#include "Scene.h"
#include "Dictionary.h"
#include "EntityId.h"
#include "Entity.h"

#include "Util.h"
#include "SparseBucketArray.h"
#include "LocalVector.h"

#include <array>
#include <bitset>
#include <functional>

// Tests to see if all set bits in one bitset are also set in another.
constexpr bool match_bitset( uint64_t bitset, uint64_t match )
{
    return (bitset & match) == match;
}

// Returns a bitset with the specified bit indices set.
template< size_t Bit >
constexpr uint64_t const_bitset( uint64_t bits = 0 )
{
    return bits | (1 << Bit);
}

// Returns a bitset with the specified bit indices set.
template< size_t Bit1, size_t Bit2, size_t... Bits >
constexpr uint64_t const_bitset( uint64_t bits = 0 )
{
    return const_bitset< Bit2, Bits... >( bits | (1 << Bit1) );
}

using Eid = int;

// A fully managed* ECS or Entity-Component-System.
// *what does it mean to be 'fully managed'?
// ?I don't know but it sounds good.
template<
    size_t      MaxEntities,
    typename... Components
>
class ComponentManager
{
public:

    // Maximum number of entities (and components of each type) to be stored.
    static constexpr int MAX_ENTITIES = MaxEntities;
    // Total number of component types.
    static constexpr int COMPONENT_COUNT = sizeof...(Components);

    // Component storage type alias.
    template< typename T, size_t SIZE = MAX_ENTITIES >
    using Storage = std::array< T, SIZE >;
    //using Storage = SparseBucketArray< T, SIZE >;

    // SparseBucketArray is more memory efficient but is slower to search.
    // It also requires allocation.

    // Aggregates component storage.
    using ComponentsCollection = std::tuple< Storage< Components >... >;

private:

    // Maps ids to entities.
    //Dictionary< EntityId, Entity > _entities { MAX_ENTITIES };
    
    LocalVector< Eid, MAX_ENTITIES > _nttIds;

    Eid _addEntity( Eid eid )
    {
        auto where = binary_search2( _nttIds, eid );
        _nttIds.insert( where, eid );
        return eid;
    }

    void _removeEntity( Eid eid )
    {
        auto where = binary_search( _nttIds, eid );
        _nttIds.remove( where );
    }

    Storage< std::bitset< COMPONENT_COUNT > > _attachments;

    // Stores all components of each type.
    ComponentsCollection _components;

    Eid _nextId = 0;

    Eid _nextEid()
    {
        return _nextId++; // @Temporary

        for ( auto itr = _nttIds.begin() + 1; itr < _nttIds.end(); ++itr )
            if ( itr[ -1 ] + 1 != *itr )
                return itr[ -1 ] + 1;

        return _nttIds.size();
    }

public:

    // Generates and returns a new entity with no attached components.
    Eid newEntity()
    {
        return _addEntity( _nextEid() );
    }

    // Detaches all components from the entity with the given id and then
    // removes it from the set of managed entities.
    void deleteEntity( Eid eid )
    {
        if ( eid == -1 )
            return;

        detachAll( eid );
        _removeEntity( eid );
    }

    // Generates and returns a new entity with the passed components
    // attached to it.
    template< typename... Components >
    Eid createEntity( Components&&... comps )
    {
        Eid eid = newEntity();
        // Hack to repeatedly call a function via pack expansion.
        auto _ = { (attach( eid, forward< Components >( comps ) ), 1)..., 0 };
        return eid;
    }

    // Attaches a component to an entity, overwriting any existing ones.
    template< typename Component >
    void attach( Eid eid, Component&& cmpt )
    {
        _attachments[ eid ][ component_index< Component >() ] = 1;
        get< Component >( eid ) = forward< Component >( cmpt );
    }

    // Tests whether an entity has a particular type of component attached.
    template< typename Component >
    bool hasAttached( Eid eid ) const
    {
        return _attachments[ eid ][ component_index< Component >() ];
    }

    // Detaches a component from an entity. The component is destroyed.
    template< typename Component >
    void detach( Eid eid )
    {
        destroy( get< Component >( eid ) );
        _attachments[ eid ][ component_index< Component >() ] = 0;
    }

    // Detaches all components attached to an entity.
    void detachAll( Eid eid )
    {
        TUPLE_FOR( auto& cmpt, _components ) {
            using Component = decltype( cmpt[ 0 ] );
            if ( hasAttached< Component >( eid ) )
                detach< Component >( eid );
        };
    }

    // Gets a component which is already attached to an entity.
    template< typename Component >
    decltype(auto) get( Eid eid )
    {
        using Type = Storage< std::decay_t< Component > >;
        assert( hasAttached< Component >( eid ) );
        return std::get< Type >( _components )[ eid ];
    }

    // Gets a component if one is attached, otherwise it calls the backup
    // function for a default result. The backup function will be called 
    // without paramaters. A backup function is provided instead of a 
    // backup value as a way to ensure the backup result is only computed 
    // if the manager fails to find an attached component.
    template< typename Component, typename BackupFn >
    decltype(auto) get( Eid eid, BackupFn&& getDefault )
    {
        return hasAttached< Component >( eid )
            ? get< Component >( eid ) : getDefault();
    }

private:

    template< typename Func, typename... Components >
    bool invokeProcess( Eid eid, Func&& func, std::tuple< Components... >* )
    {
        if ( match_signature< Components... >( eid ) )
        {
            func( get< Components >( eid )... );
            return true;
        }
        return false;
    }

    template< typename Func, typename... Components >
    void invokeSystem( Func&& func, std::tuple< Eid, Components... >* )
    {
        for ( Eid eid : _nttIds )
            if ( match_signature< Components... >( eid ) )
                func( eid, get< Components >( eid )... );
    }

    template< typename Sys >
    static constexpr bool validate_system_args()
    {
        // Tag used in overload resolution.
        using params_tag = function_traits< Sys >::decay_args_tuple;
        using EntityParam = function_traits< Sys >::arg< 0 >;
        return std::is_same< EntityParam, Eid >::value
            && validate_system( (params_tag*) 0 );
    }

    template< typename Proc >
    static constexpr bool validate_process_args()
    {
        // Tag used in overload resolution.
        using params_tag = function_traits< Proc >::decay_args_tuple;
        return validate_process( (params_tag*) 0 );
    }

protected:

    template< typename Func >
    auto invokeProcess( Eid eid, Func&& func )
        -> enable_if_t< validate_process_args< Func >(), bool >
    {
        using params_tag = function_traits< Func >::args_tuple;
        return invokeProcess( eid, func, (params_tag*) 0 );
    }

    // Invokes a function on all entities which match the signature
    // composed by the supplied Component types. The function 'updateFn' will
    // be invoked with argument types [Eid, Components&...].
    template< typename Func >
    auto invokeSystem( Func&& func )
        -> enable_if_t< validate_system_args< Func >() >
    {
        // Tag used in overload resolution.
        using params_tag = function_traits< Func >::args_tuple;
        invokeSystem( forward< Func >( func ), (params_tag*) 0 );
    }

    // Generates a signature unique to the supplied set of components.
    template< typename... Components >
    static constexpr uint64_t compose_signature()
    {
        return _compose_signature< Components... >();
    }

    // Returns true if all the bits set in the signature composed by the supplied
    // components are also set in the supplied bitset. False otherwise.
    // If this function returns true, it is said that the bitset "matches" 
    // the signature.
    template< typename... Components >
    static constexpr bool match_signature( uint64_t bitset )
    {
        return match_bitset( bitset, compose_signature< Components... >() );
    }

    // Returns true if the entity "matches" the signature composed by the supplied 
    // components. False otherwise.
    template< typename... Components >
    bool match_signature( Eid eid )
    {
        return match_signature< Components... >( _attachments[ eid ].to_ullong() );
    }

    // Convenience function allowing std::bitset to be matched against a signature.
    template< typename... Components, size_t N >
    static bool match_signature( const std::bitset< N >& bitset )
    {
        return match_signature< Components... >( bitset.to_ullong() );
    }

private:

    // Returns a zero-based index indicating where a component of a particular 
    // type is located in the manager.
    template< typename Component >
    static constexpr size_t component_index()
    {
        using Type = Storage< std::decay_t< Component > >;
        return tuple_element_index< Type, ComponentsCollection >::value;
    }

    // Implementation of non-empty component signature composition.
    template< typename... Components >
    static constexpr auto _compose_signature()
        -> std::enable_if_t< (sizeof...(Components) > 0), uint64_t >
    {
        return const_bitset< component_index< Components >()... >();
    }

    // Dummy implementation of empty component signature composition.
    template< typename... Components >
    static constexpr auto _compose_signature()
        -> std::enable_if_t< (sizeof...(Components) == 0), uint64_t >
    {
        return 0;
    }

    template< typename Last >
    static constexpr bool _validate_signature( std::tuple< Last >* )
    {
        return is_any< Last, Components... >::value;
    }

    template< typename First, typename... Rest >
    static constexpr bool _validate_signature( std::tuple< First, Rest... >* )
    {
        return is_any< First, Components... >::value
            && _validate_signature( (std::tuple< Rest... >*) 0 );
    }

    template< typename... Components >
    static constexpr bool validate_system( std::tuple< Eid, Components... >* )
    {
        return _validate_signature( (std::tuple< Components... >*) 0 );
    }

    template< typename... Components >
    static constexpr bool validate_process( std::tuple< Components... >* )
    {
        return _validate_signature( (std::tuple< Components... >*) 0 );
    }

public:

    const auto& getEntities() const
    {
        return _nttIds;
    }

    // Returns the number of entities managed by the ECS.
    int entityCount() const
    {
        return _nttIds.size();
    }

    // Returns the number of components attached to entities.
    // Note: this function operates in O(n) time.
    int componentCount() const
    {
        int count = 0;
        for ( Eid eid : _nttIds )
            count += _attachments[ eid ].count();
        return count;
    }

    // Gets the number entities which match the supplied signature.
    // Note: this function operates in O(n) time.
    template< typename... Components >
    int count() const
    {
        int count = 0;
        for ( Eid eid : _nttIds )
            count += match_signature< Components... >( eid );
        return count;
    }
};
