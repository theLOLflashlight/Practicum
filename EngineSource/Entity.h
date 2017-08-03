#pragma once

#include <bitset>

struct Entity
{
    using Bitset = std::bitset< 32 >;

    const int index;
    Bitset    bitset;

    explicit Entity( int index = 0 )
        : index( index )
    {
    }

    Entity( Entity&& ntt )
        : index( ntt.index )
        , bitset( ntt.bitset )
    {
    }

    Entity& operator =( const Entity& ntt )
    {
        // This is safe because operator= is non-const.
        const_cast< int& >( index ) = ntt.index;
        bitset = ntt.bitset;
        return *this;
    }

protected:

    template< size_t MaxEntities, typename... Components >
    friend class ComponentManager;

    Entity( const Entity& ntt )
        : index( ntt.index )
        , bitset( ntt.bitset )
    {
    }
};

inline bool operator ==( const Entity& a, const Entity& b )
{
    return a.index == b.index && a.bitset == b.bitset;
}
