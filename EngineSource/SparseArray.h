#pragma once

#include "ArrayBase.h"
#include "Util.h"

#include <cassert>
#include <bitset>

// An in-place (heapless) fixed capacity sparse array.
template< typename T, size_t N >
class SparseArray
    : private ArrayBase< T, N >
{
    // Indicates which positions in the array contain elements.
    std::bitset< N > _occupancy;

    template< typename T >
    class _iterator;

public:

    using ValueType = T;
    static constexpr size_t SIZE = N;

    // Special element return value implementation.
    class Reference;

    using Iterator = _iterator< ValueType >;
    using ConstIterator = _iterator< const ValueType >;

    SparseArray()
        : _occupancy()
    {
    }

    ~SparseArray()
    {
        for ( ValueType& value : *this )
            destroy( value );
    }

    // Returns the number of elements in the array.
    size_t size() const
    {
        return _occupancy.count();
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
        _occupancy.reset();
    }

    // Returns true if the array contains no elements.
    bool empty() const
    {
        return _occupancy.none();
    }

    // Returns true if the array has elements at all positions.
    bool full() const
    {
        return _occupancy.all();
    }

    // Indexed element access.
    // Returns a special object which behaves like a reference.
    Reference operator []( size_t i )
    {
        return { *this, i };
    }

    // Indexed const-element access.
    // Returns a special object which behaves like a reference.
    const Reference operator []( size_t i ) const
    {
        return { *this, i };
    }

    // Removes the element at a position from the array.
    void remove( size_t i )
    {
        assert( i < SIZE );
        _occupancy.set( i, false );
        _array[ i ].~ValueType();
    }

    // Behaves like a ValueType& except when it is assigned to from
    // an object of type ValueType, in which case it assigns the object
    // and updates the occupancy of the container.
    class Reference
    {
    private:

        friend class SparseArray;

        SparseArray* _pArray;
        size_t       _pos;

        Reference()
            : _pArray( nullptr ), _pos( 0 )
        {
        }

        Reference( SparseArray& array, size_t pos )
            : _pArray( &array ), _pos( pos )
        {
        }

    public:

        // Copies the object and updates occupancy.
        Reference& operator =( const ValueType& obj )
        {
            assert( _pos < SIZE );
            _pArray->_array[ _pos ] = obj;
            _pArray->_occupancy.set( _pos, true );
            return *this;
        }

        // Moves the object and updates occupancy.
        Reference& operator =( ValueType&& obj )
        {
            assert( _pos < SIZE );
            _pArray->_array[ _pos ] = std::move( obj );
            _pArray->_occupancy.set( _pos, true );
            return *this;
        }

        // Removes the element from the container.
        void remove()
        {
            _pArray->remove( _pos );
        }

        operator ValueType&()
        {
            assert( _pos < SIZE );
            assert( _pArray->_occupancy.test( _pos ) );
            return _pArray->_array[ _pos ];
        }

        operator const ValueType&() const
        {
            assert( _pos < SIZE );
            assert( _pArray->_occupancy.test( _pos ) );
            return _pArray->_array[ _pos ];
        }

        // Tests for occupancy.
        operator bool() const
        {
            assert( _pos < SIZE );
            return _pArray->_occupancy.test( _pos );
        }
    };


    #pragma region Iterators

    template< typename T >
    class _iterator
    {
        friend class SparseArray;

    public:
        using ValueType = T;

        SparseArray* _pArray;
        int _pos;

        _iterator( SparseArray& array, int pos )
            : _pArray( &array )
            , _pos( pos )
        {
        }

        // Copy ctor allows ConstIterator to be constructed from an Iterator
        // but not vice-versa.
        _iterator( const _iterator< std::remove_const_t< ValueType > >& itr )
            : _pArray( itr._pArray )
            , _pos( itr._pos )
        {
        }

        // Ensures iterator points to an occupied position.
        _iterator& init()
        {
            auto pos = _pos;

            if ( pos >= SIZE )
                return *this;

            while ( !_pArray->_occupancy.test( pos ) )
                if ( ++pos >= SIZE )
                    break;

            if ( pos < SIZE )
                _pos = pos;

            return *this;
        }

    public:

        // Removes the element from the container.
        void remove()
        {
            assert( _pos < SIZE );
            _pArray->_occupancy.set( _pos, false );
            _pArray->_array[ _pos ].~ValueType();
            operator--();
        }

        #pragma region Operators

        ValueType& operator *() const
        {
            return _pArray->_array[ _pos ];
        }

        ValueType* operator ->() const
        {
            return &_pArray->_array[ _pos ];
        }

        auto operator ++()
        {
            auto pos = _pos;

            do if ( ++pos >= SIZE )
                break;
            while ( !_pArray->_occupancy.test( pos ) );

            if ( pos <= SIZE )
                _pos = pos;
            
            return *this;
        }

        auto operator --()
        {
            auto pos = _pos;

            do if ( --pos < 0 )
                break;
            while ( !_pArray->_occupancy.test( pos ) );

            if ( pos >= 0 )
                _pos = pos;

            return *this;
        }

        auto operator ++(int)
        {
            auto tmp = *this;
            ++*this;
            return tmp;
        }

        auto operator --(int)
        {
            auto tmp = *this;
            --*this;
            return tmp;
        }

        bool operator ==( const _iterator& itr )
        {
            return _pos == itr._pos && _pArray == itr._pArray;
        }

        bool operator !=( const _iterator& itr )
        {
            return _pos != itr._pos || _pArray != itr._pArray;
        }

        #pragma endregion
    };

    #pragma region STD Iterator Functions

    using iterator = Iterator;
    using const_iterator = ConstIterator;

    iterator begin()
    {
        return Iterator( *this, 0 ).init();
    }

    iterator end()
    {
        return Iterator( *this, SIZE );
    }

    const_iterator cbegin() const
    {
        return ConstIterator( *this, 0 ).init();
    }

    const_iterator cend() const
    {
        return ConstIterator( *this, SIZE );
    }

    const_iterator begin() const
    {
        return cbegin();
    }

    const_iterator end() const
    {
        return cend();
    }

    #pragma endregion

    #pragma endregion
};
