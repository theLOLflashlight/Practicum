#pragma once

#include "TextureOffsets.h"
#include "Util.h"

#include <bitset>
#include <glm/glm.hpp>

enum AdjDirection : char
{
    WEST = 0,
    SOUTH = 1,
    EAST = 2,
    NORTH = 3,
    NORTH_WEST = 4,
    NORTH_EAST = 5,
    SOUTH_EAST = 6,
    SOUTH_WEST = 7
};

class SmartTexture
{
public:

    using Bitset = std::bitset< 8 >;

    ivec2   initialOffset;
    uint8_t mask;
    Bitset  connectAdj;

    decltype(auto) operator []( AdjDirection dir )
    {
        return connectAdj[ dir ];
    }

    bool operator []( AdjDirection dir ) const
    {
        return connectAdj.test( dir );
    }

    TextureOffset textureOffset( const TextureOffset* offsets ) const
    {
        return offsets[ connectAdj.to_ulong() & mask ];
    }

};
