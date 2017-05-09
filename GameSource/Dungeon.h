#pragma once

#include "Util.h"
#include "SmartTexture.h"

#include <memory>
#include <vector>

using Position = glm::vec2;

enum TextureId
{
    DEFAULT,

    FLOOR,
    WALL,
    PIT,

    WARRIOR,
    ROGUE,
    MAGE,
    PALADIN,
};

const glm::vec2 TEXTURE_SIZE[]
{
    { 1, 1 },
    // Tiles
    { 336, 624 },
    { 320, 816 },
    { 128, 512 },
    // Players
    { 64, 64 },
    { 64, 64 },
    { 64, 64 },
    { 64, 64 },
    // Other

};

enum class TileType
{
    NONE, FLOOR, WALL, PIT
};

struct LevelTile
{
    enum Floor
    {
        NO_FLOOR,
        TILE1, TILE2, TILE3, TILE4,
        WOOD1, WOOD2, WOOD3, WOOD4,
        DIRT1, DIRT2, DIRT3, DIRT4,
        GRASS1, GRASS2, GRASS3, GRASS4,
    };

    static ivec2 floor_offset( Floor flav )
    {
        static const ivec2 OFFSET[]
        {
            { 0, 0 },
            // TILE
            { 0, 3 },
            { 0, 6 },
            { 0, 9 },
            { 0, 12 },
            // WOOD
            { 7, 15 },
            { 7, 18 },
            { 7, 21 },
            { 7, 24 },
            // DIRT
            { 0, 15 },
            { 0, 18 },
            { 0, 21 },
            { 0, 24 },
            // GRASS
            { 7, 3 },
            { 7, 6 },
            { 7, 9 },
            { 7, 12 },
        };
        return OFFSET[ flav ];
    }

    enum Wall
    {
        NO_WALL,
        BRICK1, BRICK2, BRICK3, BRICK4,
        STONE1, STONE2, STONE3, STONE4,
        ROCK1, ROCK2, ROCK3, ROCK4,
        MINE1, MINE2, MINE3, MINE4,
        CRYSTAL1, CRYSTAL2, CRYSTAL3, CRYSTAL4,
    };

    static ivec2 wall_offset( Wall flav )
    {
        static const ivec2 OFFSET[]
        {
            { 0, 0 },
            // BRICK
            { 0, 3 },
            { 0, 6 },
            { 0, 9 },
            { 0, 12 },
            // STONE
            { 7, 15 },
            { 7, 18 },
            { 7, 21 },
            { 7, 24 },
            // ROCK
            { 0, 15 },
            { 0, 18 },
            { 0, 21 },
            { 0, 24 },
            // MINE
            { 14, 3 },
            { 14, 6 },
            { 14, 9 },
            { 14, 12 },
            // CRYSTAL
            { 14, 39 },
            { 14, 42 },
            { 14, 45 },
            { 14, 48 },
        };
        return OFFSET[ flav ];
    }

    enum Pit
    {
        NO_PIT,
        EMPTY1, EMPTY2, EMPTY3,
        ICE1, ICE2, ICE3,
        WATER1, WATER2, WATER3,
        POISON1, POISON2, POISON3,
        WAVES1, WAVES2, WAVES3,
    };

    static ivec2 pit_offset( Pit flav )
    {
        static const ivec2 OFFSET[]
        {
            { 0, 0 },
            // EMPTY
            { 0, 3 },
            { 0, 6 },
            { 0, 9 },
            // ICE
            { 0, 12 },
            { 0, 15 },
            { 0, 18 },
            // WATER
            { 0, 21 },
            { 0, 24 },
            { 0, 27 },
            // POISON
            { 0, 30 },
            { 0, 33 },
            { 0, 36 },
            // WAVES
            { 0, 39 },
            { 0, 42 },
            { 0, 45 },
        };
        return OFFSET[ flav ];
    }

    TileType tileType;
    ivec2    category;
    std::bitset< 8 > connections;

    union {
        Floor floorFlavor;
        Wall  wallFlavor;
        Pit   pitFlavor;
    };

    explicit LevelTile( TileType type = TileType::NONE,
                        ivec2 offset = { 0, 0 } )
        : tileType { type }
        , category { offset }
    {
    }

    LevelTile( Floor flav )
        : LevelTile( TileType::FLOOR, floor_offset( flav ) )
    {
        floorFlavor = flav;
    }

    LevelTile( Wall flav )
        : LevelTile( TileType::WALL, wall_offset( flav ) )
    {
        wallFlavor = flav;
    }

    LevelTile( Pit flav )
        : LevelTile( TileType::PIT, pit_offset( flav ) )
    {
        pitFlavor = flav;
    }

    decltype(auto) operator []( AdjDirection dir )
    {
        return connections[ dir ];
    }

    bool operator []( AdjDirection dir ) const
    {
        return connections.test( dir );
    }

    template< typename List >
    void updateConnections( List&& adjs )
    {
        int dir = 0;
        for ( LevelTile& tile : adjs )
            connections[ dir++ ] = tileType == tile.tileType;
    }

    TextureId getTexture() const
    {
        return (TextureId) tileType;
    }

    glm::vec4 getSprite() const
    {
        static constexpr uint8_t BITMASKS[] { 0, 15, 15, 63 };
        static constexpr TextureOffset NULL_OFFSET { 0, 0 };
        static const TextureOffset* const OFFSETS[] {
            &NULL_OFFSET, FLOOR_OFFSETS, WALL_OFFSETS, PIT_OFFSETS
        };

        int index = connections.to_ulong() & BITMASKS[ (int) tileType ];
        TextureOffset offset = OFFSETS[ (int) tileType ][ index ];
        return vec4( category + ivec2( offset.x, offset.y ), 1, 1 ) * 16f;
    }
};

class Room
{
    ivec2 size;
    std::unique_ptr< LevelTile[] > tiles;

public:

    Room( int width, int height )
        : size { width, height }
        , tiles { new LevelTile[ width * height ] }
    {
    }

    Room( const Room& copy )
        : Room( copy.width(), copy.height() )
    {
        for ( int i = 0; i < copy.numTiles(); ++i )
            tiles[ i ] = copy.tiles[ i ];
    }

    Room( Room&& ) = default;

    int numTiles() const
    {
        return size.x * size.y;
    }

    int width() const
    {
        return size.x;
    }

    int height() const
    {
        return size.y;
    }

    LevelTile& getTile( int x, int y )
    {
        return tiles[ x + y * width() ];
    }

    LevelTile* findTile( int x, int y )
    {
        if ( x < 0 || x >= width() || y < 0 || y >= height() )
            return nullptr;
        return &tiles[ x + y * width() ];
    }

    template< typename Func >
    void eachTile( Func&& func )
    {
        int i = 0;
        for ( int y = 0; y < size.y; ++y )
            for ( int x = 0; x < size.x; ++x )
                func( tiles[ i++ ], { x, y } );
    }
};

class Dungeon
{
    struct DungeonRoom : Room
    {
        vec2 pos { 0, 0 };

        DungeonRoom() : Room( 1, 1 )
        {
        }

        DungeonRoom( Room room, vec2 pos = { 0, 0 } )
            : Room( move( room ) )
            , pos { pos }
        {
        }
    };

    std::vector< DungeonRoom > rooms;

public:

    LevelTile& getTile( int x, int y )
    {
        for ( DungeonRoom& room : rooms )
            if ( x >= room.pos.x && x < room.pos.x + room.width() )
            if ( y >= room.pos.y && y < room.pos.y + room.height() )
                return room.getTile( x - room.pos.x, y - room.pos.y );

        throw "Invalid Tile";
    }

    LevelTile* findTile( int x, int y )
    {
        for ( DungeonRoom& room : rooms )
            if ( x >= room.pos.x && x < room.pos.x + room.width() )
            if ( y >= room.pos.y && y < room.pos.y + room.height() )
                return room.findTile( x - room.pos.x, y - room.pos.y );

        return nullptr;
    }

    void addRoom( Room room, vec2 pos = { 0, 0 } )
    {
        rooms.emplace_back( move( room ), pos );
    }

    template< typename Func >
    void eachRoom( Func&& func )
    {
        for ( DungeonRoom& room : rooms )
            func( (Room&) room, room.pos );
    }

    void settleRooms()
    {
        for ( DungeonRoom& room : rooms )
            room.pos = glm::round( room.pos );
    }
};
