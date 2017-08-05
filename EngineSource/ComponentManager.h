#pragma once

#include "Util.h"
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

// Entity Id type. Serves as an index into several ComponentManager sub-structures.
using Eid = int;
// Eid indicating no entitiy.
enum : Eid { NULL_EID = -1 };

enum class NoFlags { _last = 0 };

// A fully managed* ECS or Entity-Component-System.
// *what does it mean to be 'fully managed'?
// ?I don't know but it sounds good.
template< size_t MaxEntities, typename FlagsType, typename... Components >
class ComponentManager
{
public:

    using Flags = FlagsType;

    static constexpr bool HAS_FLAGS = !std::is_same_v< Flags, NoFlags >;

    // Maximum number of entities (and components of each type) to be stored.
    static constexpr int MAX_ENTITIES = MaxEntities;

    // Total number of component types.
    static constexpr int COMPONENT_COUNT = sizeof...(Components);

    using Bitset = std::bitset< COMPONENT_COUNT + Flags::_last >;
    using BitRef = typename Bitset::reference;

    static constexpr uint64_t ATTACHMENT_MASK = ~0ull >> (64 - COMPONENT_COUNT);

    // Component storage type alias.
    template< typename T, size_t SIZE = MAX_ENTITIES >
    using Storage = std::array< T, SIZE >;

    // Stores entities by their Eid.
    using EntityCollection = LocalVector< Eid, MAX_ENTITIES >;

    // Stores component attachment bitsets for each entity.
    using AttachmentCollection = Storage< Bitset >;

    // Aggregates component storage.
    using ComponentsCollection = std::tuple< Storage< Components >... >;

private:

    // Ordered set of active entities given by their Eid.
    EntityCollection     _entities;
    // List of attached components to each entity.
    AttachmentCollection _attachments;
    // Tuple of all components of each type.
    ComponentsCollection _components;

    // Returns the next eid to be returned by newEntity().
    Eid _nextEid()
    {
        // Return the index of the first element whose
        // index does not match its value.
        for ( size_t i = 0; i < _entities.size(); ++i )
            if ( _entities[ i ] != i )
                return i;

        // Return the index of _entities.end() or NULL_EID.
        return _entities.size() == _entities.capacity() ? NULL_EID : _entities.size();
    }

    // Adds an entity to the ECS and returns it.
    Eid _addEntity( Eid eid )
    {
        assert( in_range( eid, 0, MAX_ENTITIES ) );
        _entities.insert( binary_search2( _entities, eid ), eid );
        return eid;
    }

    // Removes an entity from the ECS.
    void _removeEntity( Eid eid )
    {
        if ( in_range( eid, 0, MAX_ENTITIES ) )
        {
            _attachments[ eid ].reset();
            _entities.remove( binary_search( _entities, eid ) );
        }
    }

    template< typename... Components >
    static constexpr bool validate_system( std::tuple< Eid, Components... >* = 0 )
    {
        return validate_signature< Components... >();
    }

    template< typename... Components >
    static constexpr bool validate_process( std::tuple< Components... >* = 0 )
    {
        return validate_signature< Components... >();
    }

    template< typename Component >
    static constexpr bool validate_component()
    {
        return validate_signature< Component >();
    }

public:

    // Yields the active entity ids.
    generator< Eid > entities() const
    {
        for ( Eid eid : _entities )
            co_yield eid;
    }

    // Yields the subset of entities which match the provided signature.
    template< typename... Components,
        typename = enable_if_t< sizeof...(Components) != 0 > >
    generator< tuple< Eid, Components&... > > entities()
    {
        for ( Eid eid : _entities )
            if ( match_signature< Components... >( eid ) )
                co_yield { eid, get< Components >( eid )... };
    }

    // Returns the components of an entity if it matches the provided signature.
    template< typename... Components >
    optional< tuple< Components&... > > entity( Eid eid )
    {
        if ( match_signature< Components... >( eid ) )
            return get< Components... >( eid );
        return {};
    }

    // Yields all attached components of a particular type via reference_wrapper.
    template< typename Component, typename... Guard,
        typename = enable_if_t< sizeof...(Guard) == 0
            && validate_component< Component >() > >
    auto components()
    {
        for ( Eid eid : _entities )
            if ( hasAttached< Component >( eid ) )
                co_yield std::ref( get< Component >( eid ) );
    }

    // Generates and returns a new entity with no attached components.
    Eid newEntity()
    {
        return _addEntity( _nextEid() );
    }

    // Generates and returns a new entity with the supplied components
    // attached to it.
    template< typename... Components >
    Eid newEntity( Components&&... comps )
    {
        Eid eid = newEntity();
        // Hack to repeatedly call a function via pack expansion.
        auto _ = { (attach( eid, forward< Components >( comps ) ), 1)..., 0 };
        return eid;
    }

    // Checks whether an entity is active in the manager (it has been instantiated).
    bool exists( Eid eid ) const
    {
        assert( in_range( eid, 0, MAX_ENTITIES ) );
        return binary_search( _entities, eid ) != _entities.end();
    }

    // Detaches all components from the entity with the given id and then
    // removes it from the set of managed entities.
    void deleteEntity( Eid eid )
    {
        if ( in_range( eid, 0, MAX_ENTITIES ) )
        {
            detachAll( eid );
            _removeEntity( eid );
        }
    }

    template< typename Func >
    auto deleteEntities( Func&& func )
        -> decltype( (bool) func( Eid() ), void() )
    {
        auto split = std::remove_if( _entities.begin(), _entities.end(), func );
        for ( auto itr = split; itr != _entities.end(); ++itr )
        {
            detachAll( *itr );
            _attachments[ *itr ].reset();
            *itr = 0;
        }
        _entities.resize( std::distance( _entities.begin(), split ) );
    }

    // Attaches a component to an entity, overwriting any existing ones.
    template< typename Component >
    void attach( Eid eid, Component&& cmpt )
    {
        assert( in_range( eid, 0, MAX_ENTITIES ) );
        _attachments[ eid ][ component_index< Component >() ] = 1;
        get< Component >( eid ) = forward< Component >( cmpt );
    }

    // Tests whether an entity has a particular type of component attached.
    template< typename Component >
    bool hasAttached( Eid eid ) const
    {
        assert( in_range( eid, 0, MAX_ENTITIES ) );
        return _attachments[ eid ][ component_index< Component >() ];
    }

    // Detaches a component from an entity. The component is destroyed.
    template< typename Component >
    void detach( Eid eid )
    {
        assert( in_range( eid, 0, MAX_ENTITIES ) );
        destroy( get< Component >( eid ) );
        _attachments[ eid ][ component_index< Component >() ] = 0;
    }

    // Detaches all components attached to an entity.
    void detachAll( Eid eid )
    {
        assert( in_range( eid, 0, MAX_ENTITIES ) );
        TUPLE_FOR( auto& cmpt, _components )
        {
            using Component = decltype( cmpt[ 0 ] );
            if ( hasAttached< Component >( eid ) )
                detach< Component >( eid );
        };
    }

    // Gets a component which is attached to an entity.
    template< typename Component >
    Component& get( Eid eid )
    {
        assert( hasAttached< Component >( eid ) );
        using Type = Storage< std::decay_t< Component > >;
        // Index into the storage for the type of component.
        return std::get< Type >( _components )[ eid ];
    }

    // Gets several components which are attached to an entity.
    template< typename... Components,
        typename = enable_if_t< (sizeof...(Components) > 1) > >
    tuple< Components&... > get( Eid eid )
    {
        return tie( get< Components >( eid )... );
    }

    // Gets a component if one is attached, otherwise it calls the backup
    // function for a default result. The backup function will be called 
    // without paramaters. A backup function is provided instead of a 
    // backup value as a way to ensure the backup result is only computed 
    // if the manager fails to find an attached component.
    template< typename Component, typename BackupFn >
    auto get( Eid eid, BackupFn&& getDefault )
        -> decltype( (Component&) getDefault() )
    {
        return hasAttached< Component >( eid )
            ? get< Component >( eid ) : getDefault();
    }

    auto flag( Eid eid, Flags flag )
        -> enable_if_t< HAS_FLAGS, BitRef >
    {
        assert( flag < Flags::_last );
        return _attachments[ eid ][ COMPONENT_COUNT + flag ];
    }

    template< Flags Flag >
    auto flag( Eid eid ) -> decltype( flag( eid, Flag ) )
    {
        static_assert( Flag < Flags::_last );
        return flag( eid, Flag );
    }

    template< typename... Rest >
    auto flags( Eid eid, Flags first, Rest... rest )
        -> enable_if_t< HAS_FLAGS, std::array< BitRef, sizeof...(Rest) + 1 > >
    {
        return { flag( eid, first ), flag( eid, rest )... };
    }

    template< Flags First, Flags... Rest >
    auto flags( Eid eid )
        -> std::array< BitRef, sizeof...(Rest) + 1 >
    {
        return flags( eid, First, Rest... );
    }

private:

    // Returns true if the supplied function's parameters are a subset of 
    // the component types used by the ECS; false otherwise.
    template< typename Proc >
    static constexpr bool validate_process_args()
    {
        // Tag used in overload resolution.
        using params_tag = function_traits< Proc >::decay_args_tuple;
        return validate_process( (params_tag*) 0 );
    }

    // Returns true if the supplied function's first parameter is of type Eid
    // and the remaining parameters are a subset of the component types used 
    // by the ECS; false otherwise.
    template< typename Sys >
    static constexpr bool validate_system_args()
    {
        // Tag used in overload resolution.
        using params_tag = function_traits< Sys >::decay_args_tuple;
        using EntityParam = function_traits< Sys >::arg< 0 >;
        return std::is_same< EntityParam, Eid >::value
            && validate_system( (params_tag*) 0 );
    }

    // invokeProcess(...) implementation.
    template< typename Func, typename... Components >
    bool _invokeProcess( Eid eid, Func&& func, std::tuple< Components... >* )
    {
        if ( match_signature< Components... >( eid ) )
        {
            func( get< Components >( eid )... );
            return true;
        }
        return false;
    }

    // invokeSystem(...) implementation.
    template< typename Func, typename... Components >
    void _invokeSystem( Func&& func, std::tuple< Eid, Components... >* )
    {
        for ( Eid eid : _entities )
            if ( match_signature< Components... >( eid ) )
                func( eid, get< Components >( eid )... );
    }

public:

    // Invokes a function on a specific entity if it matches the
    // signature composed by parameter types of func. The function 
    // 'func' will be invoked with argument types [Components&...].
    template< typename Func >
    auto invokeProcess( Eid eid, Func&& func )
        -> enable_if_t< validate_process_args< Func >(), bool >
    {
        assert( in_range( eid, 0, MAX_ENTITIES ) );
        // Tag used in overload resolution.
        using params_tag = function_traits< Func >::args_tuple;
        return _invokeProcess( eid, forward< Func >( func ), (params_tag*) 0 );
    }

    // Invokes a function on all entities which match the signature
    // composed by parameter types of func. The function 'func' will
    // be invoked with argument types [Eid, Components&...].
    template< typename Func >
    auto invokeSystem( Func&& func )
        -> enable_if_t< validate_system_args< Func >() >
    {
        // Tag used in overload resolution.
        using params_tag = function_traits< Func >::args_tuple;
        _invokeSystem( forward< Func >( func ), (params_tag*) 0 );
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
    static constexpr bool _match_signature( uint64_t bitset )
    {
        return match_bitset( bitset, compose_signature< Components... >() );
    }

    // Returns true if the entity "matches" the signature composed by the supplied 
    // components. False otherwise.
    template< typename... Components >
    constexpr bool match_signature( Eid eid ) const
    {
        auto bitset = _attachments[ eid ].to_ullong() & ATTACHMENT_MASK;
        return _match_signature< Components... >( bitset );
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
            && _validate_signature< Rest... >( 0 );
    }

    template< typename... Types >
    static constexpr bool validate_signature( std::tuple< Types... >* = 0 )
    {
        return _validate_signature< Types... >( 0 );
    }

public:

    // Returns the number of entities managed by the ECS.
    int entityCount() const
    {
        return _entities.size();
    }

    // Returns the number of components attached to entities.
    // Note: this function operates in O(n) time.
    int componentCount() const
    {
        int count = 0;
        for ( Eid eid : _entities )
            count += _attachments[ eid ].count();
        return count;
    }

    // Gets the number entities which match the supplied signature.
    // Note: this function operates in O(n) time.
    template< typename... Components >
    int count() const
    {
        int count = 0;
        for ( Eid eid : _entities )
            count += match_signature< Components... >( eid );
        return count;
    }
};
