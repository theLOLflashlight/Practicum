#pragma once

#include <array>
#include <initializer_list>
#include <cassert>

template< typename T, size_t Size >
struct stack_base
{
    std::array< T, Size > array;

    T* data()
    {
        return array.data();
    }

    const T* data() const
    {
        return array.data();
    }
};

template< typename T >
struct stack_base< T, 0 >
{
    constexpr T* data()
    {
        return nullptr;
    }

    constexpr const T* data() const
    {
        return nullptr;
    }
};

template< typename T, size_t LocalMax = 0 >
class stack
    : private stack_base< T, LocalMax >
{
public:
    
    using Value = T;
    static constexpr size_t LOCAL_SIZE = LocalMax;
    using Base = stack_base< Value, LOCAL_SIZE >;

    using ArrayType = std::array< Value, LOCAL_SIZE >;

private:

    Value* pValues;
    size_t capacity;
    size_t count;

    static Value* allocate( size_t count )
    {
        return std::calloc( count, sizeof( Value ) );
    }

    static void deallocate( Value* ptr )
    {
        std::free( (void*) ptr );
    }

public:

    stack()
        : Base()
        , pValues { data() }
        , capacity { LOCAL_SIZE }
        , count { 0 }
    {
    }

    explicit stack( size_t count, Value copy = {} )
        : Base()
        , pValues { count <= LOCAL_SIZE ? data() : allocate( count ) }
        , capacity { count <= LOCAL_SIZE ? LOCAL_SIZE : count }
        , count { count }
    {
        for ( size_t i = 0; i < count; ++i )
            new( pValues + i ) Value( copy );
    }

    ~stack()
    {
        if ( isAllocated() )
        {
            clear();
            deallocate( pValues );
        }
    }

private:

    void clear()
    {
        for ( size_t i = 0; i < count; ++i )
            pValues[ i ].~Value();
        count = 0;
    }

    bool isAllocated() const
    {
        return pValues != data() && pValues != nullptr;
    }

    void resize( size_t size )
    {
        size_t minSize = std::min( size, count );

        if ( size <= LOCAL_SIZE )
        {
            if ( isAllocated() )
            {
                Value* arr = data();

                for ( size_t i = 0; i < minSize; ++i )
                    arr[ i ] = std::move( pValues[ i ] );

                for ( size_t i = minSize; i < size; ++i )
                    arr[ i ] = {};

                clear();
                deallocate( pValues );
                pValues = arr;
            }
            else
            {
                for ( size_t i = minSize; i < size; ++i )
                    pValues[ i ] = {};

                for ( size_t i = size; i < count; ++i )
                    pValues[ i ].~Value();
            }
        }
        else
        {
            Value* arr = allocate( size );

            for ( size_t i = 0; i < minSize; ++i )
                new( arr + i ) Value( std::move( pValues[ i ] ) );

            for ( size_t i = minSize; i < size; ++i )
                new( arr + i ) Value();

            clear();
            if ( isAllocated() )
                deallocate( pValues );

            pValues = arr;
        }

        capacity = std::max( LOCAL_SIZE, size );
        count = size;
    }

public:

    size_t size() const
    {
        return count;
    }

    void push( Value value )
    {
        pValues[ count++ ] = std::move( value );
    }

    Value pop()
    {
        return std::move( pValues[ --count ] );
    }

    Value& peek()
    {
        return pValues[ count - 1 ];
    }

    const Value& peek() const
    {
        return pValues[ count - 1 ];
    }

    Value& first()
    {
        return pValues[ 0 ];
    }

    const Value& first() const
    {
        return pValues[ 0 ];
    }

    Value& operator []( size_t n )
    {
        assert( n < count );
        return pValues[ n ];
    }

    const Value& operator []( size_t n ) const
    {
        assert( n < count );
        return pValues[ n ];
    }

private:

    void shiftItems( Value* pStart, Value* pEnd, int64_t offset )
    {
        if ( offset < 0 )
            for ( Value* ptr = pStart; ptr != pEnd; ++ptr )
                ptr[ offset ] = std::move( *ptr );

        else if ( offset > 0 )
            for ( Value* ptr = pEnd - 1; ptr < pStart; --ptr )
                ptr[ offset ] = std::move( *ptr );
    }

public:

    Value* begin()
    {
        return pValues;
    }

    Value* end()
    {
        return pValues + count;
    }

    void insert( size_t offset, Value value )
    {
        shiftItems( begin() + offset, end(), 1 );
        pValues[ offset ] = std::move( value );
        ++count;
    }

    template< typename Itr >
    void insert( size_t offset, Itr first, Itr last )
    {
        shiftItems( begin() + offset, end(), std::distance( first, last ) );
        int i = offset;
        
        for ( Itr it = first; it != last; ++it )
            pValues[ i++ ] = *it;

        count += std::distance( first, last );
    }

};

template< typename T, size_t Size >
stack< T, Size >& operator <<( stack< T, Size >& stack, T value )
{
    stack.push( std::move( value ) );
    return stack;
}

template< typename T, size_t Size >
stack< T, Size >& operator >>( stack< T, Size >& stack, T& value )
{
    value = stack.pop();
    return stack;
}



