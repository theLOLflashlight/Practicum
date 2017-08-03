#pragma once

#include "Scene.h"
#include "Dictionary.h"
#include "EntityId.h"
#include "Entity.h"

#include "Util.h"
#include "SparseBucketArray.h"

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

// A system is a signature and a block of code to execute on entities 
// which satisfy that signature.
struct System
{
    // Invokes a function on an entity.
    using Invoker = std::function< void( Entity& ) >;

    // Defines access to the invoker.
    uint32_t signature;

    // Function indirection which supplies an entity to another function.
    Invoker  invoker;
};


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

    // Component storage type alias.
    template< typename T, size_t SIZE = MAX_ENTITIES >
    //using Storage = SparseBucketArray< T, SIZE >;
    using Storage = std::array< T, SIZE >;

    // SparseBucketArray is more memory efficient but is slower to search.
    // It also requires allocation.

    // Aggregates component storage.
    using ComponentsCollection = std::tuple< Storage< Components >... >;

private:

    // Maps ids to entities.
    Dictionary< EntityId, Entity > _entities { MAX_ENTITIES };
    
    // Stores all components of each type.
    ComponentsCollection _components;

    // Stores managed systems.
    std::vector< System > _systems;

    // EntityId and index of the next new entity.
    int _nextEntity = 0;

    std::bitset< MAX_ENTITIES > _occupancy;

    int _nextIndex()
    {
        for ( int i = 0; i < MAX_ENTITIES; ++i )
            if ( !_occupancy[ i ] )
                return i;
        #if _DEBUG
        std::cerr << "No more room for entities." << std::endl;
        #endif
        return -1;
    }

public:

    // Generates and returns a new entity with no attached components.
    Entity& newEntity()
    {
        return newEntity( (EntityId) _nextEntity++ );
    }

    // Generates and returns a new entity with no attached components.
    Entity& newEntity( EntityId eid )
    {
        int index = _nextIndex();
        _occupancy[ index ] = 1;
        return *_entities.add( eid, Entity { index } );
    }

    // Detaches all components from the entity with the given id and then
    // removes it from the set of managed entities.
    void deleteEntity( EntityId eid )
    {
        Entity& ntt = getEntity( eid );
        detachAll( ntt );
        _occupancy[ ntt.index ] = 0;
        _entities.remove( eid );
    }

    // Generates and returns a new entity with the passed components
    // attached to it.
    template< typename... Components >
    Entity& createEntity( Components&&... comps )
    {
        Entity& ntt = newEntity();
        // Hack to repeatedly call a function via pack expansion.
        auto _ = { (attach( ntt, forward< Components >( comps ) ), 1)..., 0 };
        return ntt;
    }

    // Gets the entity id of an entity.
    EntityId getId( const Entity& ntt )
    {
        return COALESCE_NULL( _entities.reverseSearch( ntt ), EntityId( ~0 ) );
    }

    // Returns a pointer to an entity with the specified eid.
    // Returns nullptr if no such entity exists.
    Entity* findEntity( EntityId eid )
    {
        return _entities.search( eid );
    }

    // Returns a pointer to an entity with the specified eid.
    // Returns nullptr if no such entity exists.
    const Entity* findEntity( EntityId eid ) const
    {
        return _entities.search( eid );
    }

    // Returns a reference to an entity with the specified eid.
    Entity& getEntity( EntityId eid )
    {
        return *_entities.search( eid );
    }

    // Returns a reference to an entity with the specified eid.
    const Entity& getEntity( EntityId eid ) const
    {
        return *_entities.search( eid );
    }

    // Attaches a component to an entity, overwriting any existing ones.
    template< typename Component >
    void attach( Entity& ntt, Component&& cmpt )
    {
        ntt.bitset[ component_index< Component >() ] = 1;
        get< Component >( ntt ) = forward< Component >( cmpt );
    }

    // Tests whether an entity has a particular type of component attached.
    template< typename Component >
    bool hasAttached( const Entity& ntt ) const
    {
        return ntt.bitset[ component_index< Component >() ];
    }

    // Detaches a component from an entity. The component is destroyed.
    template< typename Component >
    void detach( Entity& ntt )
    {
        destroy( get< Component >( ntt ) );
        ntt.bitset[ component_index< Component >() ] = 0;
    }

    // Detaches all components attached to an entity.
    void detachAll( Entity& ntt )
    {
        TUPLE_FOR( auto& cmpt, _components ) {
            using Component = decltype( cmpt[ 0 ] );
            if ( hasAttached< Component >( ntt ) )
                detach< Component >( ntt );
        };
    }

    // Gets a component which is already attached to an entity.
    template< typename Component >
    decltype(auto) get( const Entity& ntt )
    {
        using Type = Storage< std::decay_t< Component > >;
        assert( hasAttached< Component >( ntt ) );
        return std::get< Type >( _components )[ ntt.index ];
    }

    // Gets a component if one is attached, otherwise it calls the backup
    // function for a default result. The backup function will be called 
    // without paramaters. A backup function is provided instead of a 
    // backup value as a way to ensure the backup result is only computed 
    // if the manager fails to find an attached component.
    template< typename Component, typename BackupFn >
    decltype(auto) get( const Entity& ntt, BackupFn&& getDefault )
    {
        return hasAttached< Component >( ntt )
            ? get< Component >( ntt ) : getDefault();
    }

private:

    template< typename Func, typename... Components >
    bool invokeProcess( Entity& ntt, Func&& func, std::tuple< Components... >* )
    {
        if ( match_signature< Components... >( ntt ) )
        {
            func( get< Components >( ntt )... );
            return true;
        }
        return false;
    }

    template< typename Func, typename Ntt, typename... Components >
    void invokeSystem( Func&& func, std::tuple< Ntt, Components... >* )
    {
        for ( Entity& ntt : _entities )
            if ( match_signature< Components... >( ntt ) )
                func( ntt, get< Components >( ntt )... );
    }

    template< typename Sys >
    static constexpr bool validate_system_args()
    {
        // Tag used in overload resolution.
        using params_tag = function_traits< Sys >::decay_args_tuple;
        using EntityParam = function_traits< Sys >::arg< 0 >;
        return std::is_lvalue_reference< EntityParam >::value
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
    auto invokeProcess( Entity& ntt, Func&& func )
        -> enable_if_t< validate_process_args< Func >(), bool >
    {
        using params_tag = function_traits< Func >::args_tuple;
        return invokeProcess( ntt, func, (params_tag*) 0 );
    }

    // Invokes a function on all entities which match the signature
    // composed by the supplied Component types. The function 'updateFn' will
    // be invoked with argument types [Entity&, Components&...].
    template< typename Func >
    auto invokeSystem( Func&& func )
        -> enable_if_t< validate_system_args< Func >() >
    {
        // Tag used in overload resolution.
        using params_tag = function_traits< Func >::args_tuple;
        invokeSystem( forward< Func >( func ), (params_tag*) 0 );
    }

    // Invokes an Entity member function on all entities which
    // satisfy the signature composed by the member function's
    // parameters' decay types. The member function 'mbfn' will 
    // be invoked with argument types [Components&...].
    template< typename... Components >
    void invokeSystem( void (Entity::*mbfn)( Components... ) )
    {
        for ( Entity& ntt : _entities )
            if ( match_signature< Components... >( ntt ) )
                (ntt.*mbfn)( get< Components >( ntt )... );
    }

    // Adds a functor to the collection of managed systems with the 
    // signature composed by the supplied Component types.
    // This function can accept Entity member functions as an argument.
    template< typename... Components, typename Func >
    void addSystem( Func&& func )
    {
        _systems.push_back( {
            compose_signature< Components... >(),
            [this, func = forward< Func >( func )]( Entity& ntt ) {
                std::invoke( func, ntt, get< Components >( ntt )... );
            }
        } );
    }

    // Removes all systems from the collection of managed systems which 
    // match the signature composed by the supplied Component types.
    template< typename... Components >
    void removeSystems()
    {
        auto split = std::remove_if(
            _systems.begin(), _systems.end(),
            []( System& sys ) {
                return !match_signature< Components... >( sys.signature );
            } );

        _systems.erase( split, _systems.end() );
    }

    // Invokes all managed systems on corresponding entities.
    // Note: This loops over entities and then over systems.
    void invokeManagedSystems()
    {
        for ( Entity& ntt : _entities )
            for ( System& system : _systems )
                if ( match_bitset( ntt.bitset.to_ulong(), system.signature ) )
                    system.invoker( ntt );
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
    static bool match_signature( const Entity& ntt )
    {
        return match_signature< Components... >( ntt.bitset.to_ullong() );
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

    template< typename Ntt, typename... Components >
    static constexpr bool validate_system( std::tuple< Ntt, Components... >* )
    {
        return std::is_same< Entity, std::decay_t< Ntt > >::value
            && _validate_signature( (std::tuple< Components... >*) 0 );
    }

    template< typename... Components >
    static constexpr bool validate_process( std::tuple< Components... >* )
    {
        return _validate_signature( (std::tuple< Components... >*) 0 );
    }

public:

    const auto& getEntities() const
    {
        return _entities;
    }

    // Returns the number of entities managed by the ECS.
    int entityCount() const
    {
        return _entities.count();
    }

    // Returns the number of components attached to entities.
    // Note: this function operates in O(n) time.
    int componentCount() const
    {
        int count = 0;
        for ( const Entity& ntt : _entities )
            count += ntt.bitset.count();
        return count;
    }

    // Gets the number entities which match the supplied signature.
    // Note: this function operates in O(n) time.
    template< typename... Components >
    int count() const
    {
        int count = 0;
        for ( const Entity& ntt : _entities )
            count += match_signature< Components... >( ntt );
        return count;
    }
};
