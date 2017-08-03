#pragma once

#include "ArrayBase.h"
#include "Util.h"

#include <cassert>

// An in-place (heapless) fixed capacity vector.
template< typename T, size_t N >
class LocalVector
    : private ArrayBase< T, N >
{
    // Number of stored elements.
    int _count;

public:

    using ValueType = T;
    static constexpr int SIZE = N;

    LocalVector()
        : _count( 0 )
    {
    }

    LocalVector( const LocalVector& copy ) = default;

    LocalVector( LocalVector&& moved )
        : _count( moved._count )
    {
        int i = 0;
        for ( auto&& o : move( moved ) )
            _array[ i++ ] = move( o );
        moved._count = 0;
    }

    ~LocalVector()
    {
        for ( ValueType& value : *this )
            destroy( value );
        _count = 0;
    }

    LocalVector& operator =( const LocalVector& copy ) = default;

    LocalVector& operator =( LocalVector&& moved )
    {
        _count = moved._count;
        int i = 0;
        for ( auto&& o : move( moved ) )
            _array[ i++ ] = move( o );
        moved._count = 0;
        return *this;
    }

    // Returns the number of elements in the array.
    size_t size() const
    {
        return _count;
    }

    // Returns the maximum size of the array.
    size_t capacity() const
    {
        return SIZE;
    }

    // Empties the array.
    void clear()
    {
        for ( ValueType& value : *this )
            destroy( value );
        _count = 0;
    }

    // Resizes the array (not greater than capacity).
    void resize( size_t newSize )
    {
        assert( newSize <= SIZE );

        if ( newSize < _count )
            for ( int i = _count; i > newSize; --i )
                destroy( _array[ i - 1 ] );

        else if ( newSize > _count )
            for ( int i = _count; i < newSize; ++i )
                _array[ i ] = {};

        _count = newSize;
    }

    void insert( decltype( _array.begin() ) where, ValueType value )
    {
        for ( auto itr = end() - 1; itr >= where; --itr )
            itr[ 1 ] = move( *itr );

        *where = move( value );
        ++_count;
    }

    void remove( decltype( _array.begin() ) where )
    {
        for ( auto itr = where; itr != end(); ++itr )
            *itr = move( itr[ 1 ] );

        if ( where != end() )
            --_count;
    }

    // Indexed element access.
    ValueType& operator []( size_t i )
    {
        assert( i < size() );
        return _array[ i ];
    }

    // Indexed const-element access.
    const ValueType& operator []( size_t i ) const
    {
        assert( i < size() );
        return _array[ i ];
    }

    // Appends an element to the array.
    void push_back( const ValueType& value )
    {
        assert( _count < SIZE );
        _array[ _count++ ] = value;
    }

    // Removes the last element from the array.
    void pop_back()
    {
        assert( _count > 0 );
        _array[ --_count ].~ValueType();
    }

    // Gets the first element in the array.
    ValueType& front()
    {
        assert( _count > 0 );
        return _array[ 0 ];
    }

    // Gets the last element in the array.
    ValueType& back()
    {
        assert( _count > 0 );
        return _array[ _count - 1 ];
    }

    #pragma region iterator

    auto begin()
    {
        return std::begin( _array );
    }

    auto begin() const
    {
        return std::begin( _array );
    }

    auto cbegin() const
    {
        return std::cbegin( _array );
    }

    auto end()
    {
        return begin() + _count;
    }

    auto end() const
    {
        return begin() + _count;
    }

    auto cend() const
    {
        return cbegin() + _count;
    }


    auto rend()
    {
        return std::rend( _array );
    }

    auto rend() const
    {
        return std::rend( _array );
    }

    auto crend() const
    {
        return std::crend( _array );
    }

    auto rbegin()
    {
        return rend() - _count;
    }

    auto rbegin() const
    {
        return rend() - _count;
    }

    auto crbegin() const
    {
        return crend() - _count;
    }

    #pragma endregion
};
