#pragma once

#include <tuple>
#include <array>
#include <vector>
#include <algorithm>
#include <utility>
#include <memory>
#include <functional>
#include <glm/glm.hpp>

using std::forward;
using std::move;
using std::enable_if_t;

using glm::uint;
//#define uint unsigned int
using ulong = unsigned long;

using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::ivec2;
using glm::ivec3;
using glm::ivec4;

#define FORWARD( ARG ) \
std::forward< std::remove_reference_t< decltype( ARG ) > >( ARG )

#define COALESCE_NULL( MAYBE_NULL, NOT_NULL ) \
[&]( auto&& ptr ) { return !!ptr ? *ptr : NOT_NULL; }( MAYBE_NULL )

const auto fn_always_true = []( auto&&... ) { return true; };
const auto fn_always_false = []( auto&&... ) { return false; };

const auto interp_linear = []( auto&& a, auto&& b, float frac ) {
    return a + (frac * (b - a));
};

struct interp_linear_t
{
    template< typename A, typename B >
    auto operator ()( A&& a, B&& b, float frac )
    {
        return a + (frac * (b - a));
    }
};

template< typename T, size_t SIZE >
using RefArray = std::array< std::reference_wrapper< T >, SIZE >;

template< typename T >
using RefVector = std::vector< std::reference_wrapper< T > >;

template< typename Cont1, typename Cont2 >
void array_cat( Cont1& cont1, Cont2&& cont2 )
{
    cont1.insert( std::end( cont1 ),
        std::begin( forward< Cont2 >( cont2 ) ),
        std::end( forward< Cont2 >( cont2 ) ) );
}

template< typename Cont, typename Pred >
void array_remove( Cont& cont, Pred&& pred )
{
    auto split = std::remove_if(
        std::begin( cont ), std::end( cont ),
        forward< Pred >( pred ) );

    cont.erase( split, std::end( cont ) );
}

inline glm::vec2 midpoint( glm::vec2 a, glm::vec2 b )
{
    return (a + b) / 2.f;
}

inline glm::vec2 flip_x( glm::vec2 v )
{
    return { -v.x, v.y };
}

inline glm::vec2 flip_y( glm::vec2 v )
{
    return { v.x, -v.y };
}

template< typename T >
constexpr T max( T a, T b )
{
    return a < b ? b : a;
}

template< typename T >
constexpr T min( T a, T b )
{
    return a > b ? b : a;
}

template< typename T >
constexpr T clamp( T x, T min, T max )
{
    return ::min( ::max( x, min ), max );
}

struct Rect
{
    float x, y;
    float width, height;

    operator glm::vec4() const
    {
        return { x, y, width, height };
    }

    float left() const
    {
        return x;
    }

    float top() const
    {
        return y;
    }

    float right() const
    {
        return x + width;
    }

    float bottom() const
    {
        return y + height;
    }

    bool clip( glm::vec2 p ) const
    {
        return p.x < x || p.x > x + width
            || p.y < y || p.y > y + height;
    }
};

// Calls the destructor of the passed object.
template< typename T >
void destroy( T& t )
{
    t.~T();
}

// Linearly moves one value towards a destination without going over
template <typename Comparable>
Comparable approach(Comparable start, Comparable end, Comparable maxChange)
{
	if (start < end)
		return ::min(start + maxChange, end);
	else
		return ::max(start - maxChange, end);
}

// Scales a point (x) by performing the same transformation needed
// to convert the range [minA, maxA] to [minB, maxB].
template< typename T >
constexpr T scale_point( T x, T minA, T maxA, T minB, T maxB )
{
    return ((x - minA) * ((maxB - minB) / (maxA - minA))) + minB;
}

// Scales a point (x) by performing the same transformation needed
// to convert the range [minA, maxA] to [minB, maxB] and clamps 
// the result to the range [minB, maxB].
template< typename T >
constexpr T scale_point_clamp( T x, T minA, T maxA, T minB, T maxB )
{
    return clamp( scale_point( x, minA, maxA, minB, maxB ), minB, maxB );
}

// Returns a random integer between two values
inline int randomRange(int min, int max)
{
	return min + (rand() % (int)(max - min + 1));
}

// Returns a random real number between two values
inline float random( float min, float max )
{
    // between 0 and 1
    float r = rand() / (float) RAND_MAX;
    return min + (r * (max - min));
}

template< typename Itr >
Itr binary_search( Itr first, Itr last, const decltype( *first ) target )
{
    auto lower = first;
    auto upper = last;

    while ( lower <= upper )
    {
        auto midle = lower + (upper - lower) / 2;

        if ( *midle < target )
            lower = midle + 1;
        else if ( target < *midle )
            upper = midle - 1;
        else
            return midle;
    }

    return last;
}

template< typename Cont, typename T >
auto binary_search( Cont& cont, const T& target )
{
    return binary_search( std::begin( cont ), std::end( cont ), target );
}

template< typename T >
constexpr T sum( T&& arg )
{
    return arg;
}

template< typename T, typename... Ts >
constexpr T sum( T&& arg, Ts&&... args )
{
    return forward< T >( arg ) + sum( forward< Ts >( args )... );
}

template< typename T, typename... Rest >
void rotate( T& first, Rest&... rest )
{
    constexpr int SIZE = sizeof...(Rest);

    T* ptrArray[] { &first, &rest... };
    T tmp = move( *ptrArray[ SIZE ] );

    for ( int i = SIZE; i > 0; --i )
        *ptrArray[ i ] = move( *ptrArray[ i - 1 ] );

    first = move( tmp );
}

template< typename T, typename... Rest >
void unrotate( T& first, Rest&... rest )
{
    constexpr int SIZE = sizeof...(Rest);

    T* ptrArray[] { &first, &rest... };
    T tmp = move( first );

    for ( int i = 0; i < SIZE; ++i )
        *ptrArray[ i ] = move( *ptrArray[ i + 1 ] );

    *ptrArray[ SIZE ] = move( tmp );
}

template< typename T >
void rotate( T& a, T& b )
{
    std::swap( a, b );
}

template< typename T >
void unrotate( T& a, T& b )
{
    std::swap( a, b );
}

template< typename T >
void rotate( T& )
{
}

template< typename T >
void unrotate( T& )
{
}


template< typename T, typename Tuple >
struct tuple_element_index;

template< typename T, typename... Types >
struct tuple_element_index< T, std::tuple< T, Types... > >
{
    static constexpr std::size_t value = 0;
};

template < typename T, typename U, typename... Types >
struct tuple_element_index< T, std::tuple< U, Types... > >
{
    static constexpr std::size_t value
        = 1 + tuple_element_index< T, std::tuple< Types... > >::value;
};

// Function object specialization.
template< typename Func >
struct function_traits
    : function_traits< decltype( &Func::operator() ) >
{
};

// Const member function specialization.
template< typename ClassType, typename ReturnType, typename... Args >
struct function_traits< ReturnType(ClassType::*)( Args... ) const >
    : function_traits< ReturnType( Args... ) >
{
};

// Non-const member function specialization.
template< typename ClassType, typename ReturnType, typename... Args >
struct function_traits< ReturnType(ClassType::*)( Args... ) >
    : function_traits< ReturnType( Args... ) >
{
};

// Pointer to function specialization.
template< typename ReturnType, typename... Args >
struct function_traits< ReturnType(*)( Args... ) >
    : function_traits< ReturnType( Args... ) >
{
};

// Plain function specialization.
template< typename ReturnType, typename... Args >
struct function_traits< ReturnType( Args... ) >
{
    enum { ARITY = sizeof...(Args) };

    using result_type = ReturnType;

    using args_tuple = std::tuple< Args... >;

    using decay_args_tuple = std::tuple< std::decay_t< Args >... >;

    template< size_t I >
    using arg = std::tuple_element_t< I, args_tuple >;
};

template< typename T, typename... Rest >
struct is_any : std::false_type {};

template< typename T, typename First >
struct is_any< T, First > : std::is_same< T, First > {};

template< typename T, typename First, typename... Rest >
struct is_any< T, First, Rest... >
    : std::integral_constant< bool,
        std::is_same< T, First >::value || is_any< T, Rest... >::value >
{
};

template< typename Range, typename Func >
bool none_of( Range&& range, Func&& func )
{
    for ( auto&& elem : range )
        if ( (bool) std::invoke( func, elem ) )
            return false;
    return true;
}

template< typename Range, typename Class, typename Return >
bool none_of( Range&& range, Return (Class::*memberFn)() const )
{
    for ( Class& elem : range )
        if ( (bool) std::invoke( memberFn, elem ) )
            return false;
    return true;
}

template< typename Range, typename Class, typename Return >
bool none_of( Range&& range, Return Class::*member )
{
    for ( Class& elem : range )
        if ( (bool) (elem.*member) )
            return false;
    return true;
}

template< typename Range, typename Func >
bool any_of( Range&& range, Func&& func )
{
    for ( auto&& elem : range )
        if ( std::invoke( func, elem ) )
            return true;
    return false;
}

template< typename Range, typename Class, typename Return >
bool any_of( Range&& range, Return (Class::*memberFn)() const )
{
    for ( Class& elem : range )
        if ( (bool) std::invoke( memberFn, elem ) )
            return true;
    return false;
}

template< typename Range, typename Class, typename Return >
bool any_of( Range&& range, Return Class::*member )
{
    for ( Class& elem : range )
        if ( (bool) (elem.*member) )
            return true;
    return false;
}

template< typename T >
bool all( const T& t, std::initializer_list< T > il )
{
    for ( T tt : il )
        if ( t != tt )
            return false;
    return true;
}

template< typename T >
bool none( const T& t, std::initializer_list< T > il )
{
    for ( T tt : il )
        if ( t == tt )
            return false;
    return true;
}


template< size_t N, typename... Args >
decltype(auto) get_n( Args&&... args )
{
    return std::get< N >( std::forward_as_tuple( forward< Args >( args )... ) );
}


namespace detail
{
    template< typename T >
    void* get_ptr( const std::unique_ptr< T >& mem )
    {
        return mem.get();
    }

    template< typename T >
    void* get_ptr( const std::shared_ptr< T >& mem )
    {
        return mem.get();
    }

    template< typename T >
    void* get_ptr( const T& mem )
    {
        return static_cast< void* >( mem );
    }
}

template< typename AllocFn, typename... Types, typename... Sizes >
auto jalloc( AllocFn&& allocate, std::tuple< Types*&... > output, Sizes... sizes )
    -> enable_if_t< sizeof...(Types) == sizeof...(Sizes), decltype(allocate( 1 )) >
{
    size_t totalSize = sum( size_t( sizes ) * sizeof( Types )... );
    size_t blockSizes[] { size_t( sizes ) * sizeof( Types )... };

    auto allocation = allocate( totalSize );
    void* pointers[ sizeof...(Types) ] { detail::get_ptr( allocation ) };

    for ( int i = 1; i < sizeof...(Types); ++i )
        pointers[ i ] = (char*) pointers[ i - 1 ] + blockSizes[ i - 1 ];

    tuple_for( output, [&, i = 0]( auto*& ptr ) mutable {
        ptr = (decltype( ptr )) pointers[ i++ ];
    } );

    return allocation;
}

using jalloc_t = std::unique_ptr< char[] >;

template< typename... Types, typename... Sizes >
auto jalloc( std::tuple< Types*&... > output, Sizes... sizes )
    -> enable_if_t< sizeof...(Types) == sizeof...(Sizes), jalloc_t >
{
    return jalloc( std::make_unique< char[] >, output, sizes... );
    //return jalloc( std::malloc, output, sizes... );
}

template< typename... Types, typename AllocType, typename... Sizes >
auto jalloc( const std::function< AllocType( size_t ) >& allocate, Sizes... sizes )
    -> enable_if_t< sizeof...(Types) == sizeof...(Sizes), std::tuple< AllocType, Types*... > >
{
    size_t totalSize = sum( size_t( sizes ) * sizeof( Types )... );
    size_t blockSizes[] { size_t( sizes ) * sizeof( Types )... };

    AllocType allocation = allocate( totalSize );
    void* pointers[ sizeof...(Types) ] { detail::get_ptr( allocation ) };

    for ( int i = 1; i < sizeof...(Types); ++i )
        pointers[ i ] = (char*) pointers[ i - 1 ] + blockSizes[ i - 1 ];

    std::tuple< AllocType, Types*... > output;
    std::get< 0 >( output ) = move( allocation );

    tuple_for< 1 >( output, [&, i = 0]( auto*& ptr ) mutable {
        ptr = (decltype( ptr )) pointers[ i++ ];
    } );

    return output;
}

template< typename... Types, typename... Sizes >
auto jalloc( Sizes... sizes )
    -> enable_if_t< sizeof...(Types) == sizeof...(Sizes), std::tuple< jalloc_t, Types*... > >
{
    return jalloc< Types... >( std::make_unique< char[] >, sizes... );
    //return jalloc( std::malloc, output, sizes... );
}

// Empty tuple case.
template< typename Func >
void tuple_for( const std::tuple<>&, Func&& )
{
}

template< size_t N, typename... Types, typename Func >
auto tuple_for( const std::tuple< Types... >&, Func&& )
    -> std::enable_if_t< N == sizeof...(Types) >
{
}

// Calls a function on each element in a tuple.
template< size_t N = 0, typename... Types, typename Func >
auto tuple_for( const std::tuple< Types... >& tup, Func&& func )
    -> std::enable_if_t< N < sizeof...(Types) >
{
    std::invoke( forward< Func >( func ), std::get< N >( tup ) );
    tuple_for< N + 1 >( tup, forward< Func >( func ) );
}

// Calls a function on each element in a tuple.
template< size_t N = 0, typename... Types, typename Func >
auto tuple_for( std::tuple< Types... >& tup, Func&& func )
    -> std::enable_if_t< N < sizeof...(Types) >
{
    std::invoke( forward< Func >( func ), std::get< N >( tup ) );
    tuple_for< N + 1 >( tup, forward< Func >( func ) );
}

template< typename... Types, typename Func >
void operator *( std::tuple< Types... >& tup, Func&& func )
{
    tuple_for( tup, forward< Func >( func ) );
}

template< typename... Types, typename Func >
void operator *( std::tuple< Types... >&& tup, Func&& func )
{
    tuple_for( forward< std::tuple< Types... > >( tup ), forward< Func >( func ) );
}

#define TUPLE_FOR( PARAM, TUPLE ) (TUPLE) * [&]( PARAM )
//#define TUPLE_FOR( PARAM, TUPLE, ... ) tuple_for( TUPLE, [&]( PARAM ) __VA_ARGS__ )