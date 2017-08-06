#pragma once

#include <iostream>
#include <cassert>
#include <initializer_list>
#include <tuple>
#include <sstream>
#include <type_traits>

// Empty tuple case.
template< typename Func >
void for_each( const std::tuple<>&, Func&& )
{
}

// After last element case.
template< size_t N, typename... Types, typename Func >
auto for_each( const std::tuple< Types... >&, Func&& )
    -> std::enable_if_t< N == sizeof...(Types) >
{
}

// General case.
template< size_t N = 0, typename... Types, typename Func >
auto for_each( const std::tuple< Types... >& tup, Func&& func )
    -> std::enable_if_t< N < sizeof...(Types) >
{
    func( std::get< N >( tup ) );
    for_each< N + 1 >( tup, func );
}

// General case.
template< size_t N = 0, typename... Types, typename Func >
auto for_each( std::tuple< Types... >& tup, Func&& func )
    -> std::enable_if_t< N < sizeof...(Types) >
{
    func( std::get< N >( tup ) );
    for_each< N + 1 >( tup, func );
}

class String
{
    static constexpr size_t COUNT_SIZE = sizeof( int );

    void*  allocation = nullptr;
    char*  start = nullptr;
    size_t count = 0;

    int& refCount()
    {
        return *static_cast< int* >( allocation );
    }

    char* charData()
    {
        return static_cast< char* >( allocation ) + COUNT_SIZE;
    }

    String( void* allocation, char* start, size_t count )
        : allocation { allocation }
        , start { start }
        , count { count }
    {
        if ( isAllocated() )
            ++refCount();
    }

public:

    String() = default;

    String( const char* str, size_t length )
        : allocation { calloc( COUNT_SIZE + length, 1 ) }
        , start { charData() }
        , count { length }
    {
        refCount() = 1;
        if ( str != nullptr )
            memcpy( start, str, length );
    }

    template< size_t N >
    String( const char (&str)[ N ] )
        : String( str, N )
    {
    }

    String( const char* str )
        : String( str, strlen( str ) )
    {
    }

    String( const std::string& str )
        : String( str.c_str(), str.length() )
    {
    }

    String( const String& copy )
        : String( copy.allocation, copy.start, copy.count )
    {
    }

    String( String&& tmp )
        : allocation { tmp.allocation }
        , start { tmp.start }
        , count { tmp.count }
    {
        tmp.allocation = nullptr;
    }

    ~String()
    {
        if ( isAllocated() && --refCount() == 0 )
            free( allocation );
    }

    String& operator =( const String& copy )
    {
        this->~String();
        allocation = copy.allocation;
        start = copy.start;
        count = copy.count;
        return *this;
    }

    String& operator =( String&& tmp )
    {
        std::swap( allocation, tmp.allocation );
        std::swap( start, tmp.start );
        std::swap( count, tmp.count );
        return *this;
    }

    size_t length() const
    {
        return count;
    }

    void resize( size_t length )
    {
        count = length;
    }

    char* data()
    {
        return start;
    }

    const char* data() const
    {
        return start;
    }

    decltype(auto) operator []( size_t i )
    {
        assert( i < count );
        return start[ i ];
    }

    decltype(auto) operator []( size_t i ) const
    {
        assert( i < count );
        return start[ i ];
    }

    String clone( ptrdiff_t off = 0, int len = -1 ) const
    {
        assert( off + len <= count );
        return { start + off, len == -1 ? count - off : len };
    }

    String substr( ptrdiff_t off, int len = -1 ) const
    {
        assert( off + len <= count );
        return { allocation, start + off, len == -1 ? count - off : len };
    }

    size_t find_first_of( char match, size_t pos = 0 ) const
    {
        for ( auto* it = begin() + pos; it < end(); ++it )
            if ( *it == match )
                return it - begin();
        return -1;
    }

    size_t find_first_not_of( char match, size_t pos = 0 ) const
    {
        for ( auto* it = begin() + pos; it < end(); ++it )
            if ( *it != match )
                return it - begin();
        return -1;
    }

    template< typename... Args >
    String format( Args&&... args ) const
    {
        std::ostringstream oss;

        // Avoid string copy due to string::substr.
        auto output_substr = [&]( size_t first, size_t count )
        {
            auto p = begin() + first;
            auto q = first + count < length()
                ? begin() + first + count
                : end();

            for ( ; p < q; ++p )
                oss << *p;
        };

        // Avoid string copy due to string::substr.
        auto match_sentinel = [&]( size_t off ) {
            return start[ off ] == '%' && start[ off + 1 ] == '%';
        };

        size_t begin;
        size_t end = -1;

        auto inc_token = [&] {
            // perform increment
            begin = end + 1;
            end = find_first_of( '%', begin );

            // convert \% to %
            while ( end != -1 && match_sentinel( end ) )
            {
                output_substr( begin, end - begin + 1 );

                begin = end + 2;
                end = find_first_of( '%', begin );
            }
        };

        // This is where the magic actually happens.

        for_each( std::tie( args... ), [&]( auto& arg )
        {
            inc_token();
            output_substr( begin, end - begin );
            oss << arg;
        } );

        inc_token();
        output_substr( begin, end - begin );

        return oss.str();
    }

    bool isAllocated() const
    {
        return allocation != nullptr;
    }

    #pragma region Iterators

    char* begin()
    {
        return start;
    }

    char* end()
    {
        return start + count;
    }

    const char* cbegin() const
    {
        return start;
    }

    const char* cend() const
    {
        return start + count;
    }

    const char* begin() const
    {
        return cbegin();
    }

    const char* end() const
    {
        return cend();
    }

    #pragma endregion

    #pragma region Operators

    String& operator +=( ptrdiff_t dif )
    {
        start += dif;
        return *this;
    }

    String& operator -=( ptrdiff_t dif )
    {
        start -= dif;
        return *this;
    }

    String& operator ++()
    {
        ++start;
        return *this;
    }

    String operator ++( int )
    {
        auto tmp = *this;
        ++*this;
        return tmp;
    }

    String& operator --()
    {
        --start;
        return *this;
    }

    String operator --( int )
    {
        auto tmp = *this;
        --*this;
        return tmp;
    }

    #pragma endregion

};

extern std::ostream& operator <<( std::ostream& out, const String& str );

extern std::istream& operator >>( std::istream& in, String& str );

inline String operator +( const String& a, const String& b )
{
    String str( nullptr, a.length() + b.length() );
    memcpy( str.data(), a.data(), a.length() );
    memcpy( str.data() + a.length(), b.data(), b.length() );
    return str;
}

inline void reverse( String& str )
{
    for ( size_t l = 0, r = str.length() - 1; l < r; ++l, --r )
        std::swap( str[ l ], str[ r ] );
}
