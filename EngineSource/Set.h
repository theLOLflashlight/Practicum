#pragma once

#include <vector>
#include <set>
#include "Util.h"

template< typename T >
class Set
{
private:

    std::vector< T > _array;

public:

    using Value = T;

    void add( Value value )
    {
        _array.insert( binary_search( _array, value ), move( value ) );
    }

    void remove( const Value& value )
    {
        _array.erase( binary_search( _array, value ) );
    }

    bool contains( const Value& value ) const
    {
        return binary_search( _array, value ) != _array.end();
    }

    auto begin()
    {
        return _array.begin();
    }

    auto end()
    {
        return _array.end();
    }
};
