#pragma once

#include "Util.h"
#include "SmartTexture.h"
#include "Astar.h"

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

    FONT1,

    ELEMENTAL,
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
    // Fonts
    { 855, 9 },
    // Enemies
    { 128, 176 },
    // Other

};

enum class Tile
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
        ACID1, ACID2, ACID3,
        FIRE1, WATER4, ACID4,
    };

    static ivec2 pit_offset( Pit flav )
    {
        static const ivec2 OFFSET[]
        {
            { 0, 0 },
            // EMPTY
            { 0, 2 },
            { 0, 4 },
            { 0, 6 },
            // ICE
            { 0, 8 },
            { 0, 10 },
            { 0, 12 },
            // WATER
            { 0, 14 },
            { 0, 16 },
            { 0, 18 },
            // POISON
            { 0, 20 },
            { 0, 22 },
            { 0, 24 },
            // WAVES
            { 0, 26 },
            { 0, 28 },
            { 0, 30 },
        };
        return OFFSET[ flav ];
    }

    Tile tileType;
    ivec2 offset;
    std::bitset< 8 > connections;

    union {
        Floor floorFlavor;
        Wall  wallFlavor;
        Pit   pitFlavor;
    };

    explicit LevelTile( Tile type = Tile::NONE,
                        ivec2 offset = { 0, 0 } )
        : tileType { type }
        , offset { offset }
    {
    }

    LevelTile( Floor flav )
        : LevelTile( Tile::FLOOR, floor_offset( flav ) )
    {
        floorFlavor = flav;
    }

    LevelTile( Wall flav )
        : LevelTile( Tile::WALL, wall_offset( flav ) )
    {
        wallFlavor = flav;
    }

    LevelTile( Pit flav )
        : LevelTile( Tile::PIT, pit_offset( flav ) )
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

    bool allConnects( std::initializer_list< AdjDirection > il ) const
    {
        for ( AdjDirection dir : il )
            if ( !connections.test( dir ) )
                return false;
        return true;
    }

    bool noneConnects( std::initializer_list< AdjDirection > il ) const
    {
        for ( AdjDirection dir : il )
            if ( connections.test( dir ) )
                return false;
        return true;
    }

    bool shouldConnect( const LevelTile& tile ) const
    {
        if ( tileType != Tile::FLOOR && tile.tileType == Tile::NONE  )
            return true;
        return tileType == tile.tileType;
    }

    template< typename List >
    void updateConnections( List&& adjs )
    {
        int dir = 0;
        for ( const LevelTile& tile : adjs )
            connections[ dir++ ] = shouldConnect( tile );
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
        TextureOffset off = OFFSETS[ (int) tileType ][ index ];
        return vec4( offset + ivec2( off.x, off.y ), 1, 1 ) * 16f;
    }
};

namespace std
{
    template<>
    struct hash< std::reference_wrapper< LevelTile > >
    {
        size_t operator ()( const LevelTile& tile ) const
        {
            return (size_t) &tile;
        }
    };
}

inline bool operator ==( const LevelTile& lhs, const LevelTile& rhs )
{
    return &lhs == &rhs;
}

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
        for ( int i = 0; i < numTiles(); ++i )
            tiles[ i ] = copy.tiles[ i ];
    }

    Room( Room&& ) = default;

    Room& operator =( const Room& copy )
    {
        size = copy.size;
        tiles = std::make_unique< LevelTile[] >( numTiles() );

        for ( int i = 0; i < numTiles(); ++i )
            tiles[ i ] = copy.tiles[ i ];

        return *this;
    }

    Room& operator =( Room&& ) = default;

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

    bool owns( const LevelTile* pTile ) const
    {
        auto* pTiles = tiles.get();
        return pTile >= pTiles && pTile < pTiles + numTiles(); 
    }

    ivec2 tilePos( const LevelTile* pTile ) const
    {
        if ( !owns( pTile ) )
            return {};

        int off = pTile - tiles.get();
        return { off % width(), off / width() };
    }

    template< typename Func >
    void eachTile( Func&& func )
    {
        int i = 0;
        for ( int y = 0; y < size.y; ++y )
            for ( int x = 0; x < size.x; ++x )
                func( tiles[ i++ ], { x, y } );
    }

    auto enumerate()
    {
        int i = 0;
        for ( int y = 0; y < size.y; ++y )
            for ( int x = 0; x < size.x; ++x )
                co_yield pass( tiles[ i++ ], ivec2( x, y ) );
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

        DungeonRoom( const Room& room, vec2 pos = { 0, 0 } )
            : Room( room )
            , pos { pos }
        {
        }
    };

    std::vector< DungeonRoom > rooms;

    ivec2 _pos;
    ivec2 _size;

public:

    size_t roomCount() const
    {
        return rooms.size();
    }

    int calculateWidth() const
    {
        int maxWidth = 0;

        eachRoom( [&]( const Room& room, vec2 rpos )
        {
            int width = room.width() + rpos.x;
            if ( width > maxWidth )
                maxWidth = width;
        } );

        return maxWidth;
    }

    int calculateHeight() const
    {
        int maxHeight = 0;

        eachRoom( [&]( const Room& room, vec2 rpos )
        {
            int height = room.height() + rpos.y;
            if ( height > maxHeight )
                maxHeight = height;
        } );

        return maxHeight;
    }

    ivec2 calculateDimensions() const
    {
        int maxWidth = 0;
        int maxHeight = 0;

        eachRoom( [&]( const Room& room, vec2 rpos )
        {
            int width = room.width() + rpos.x;
            if ( width > maxWidth )
                maxWidth = width;

            int height = room.height() + rpos.y;
            if ( height > maxHeight )
                maxHeight = height;
        } );

        return { maxWidth, maxHeight };
    }

    int calculateX() const
    {
        int minX = 0;

        eachRoom( [&]( const Room& room, vec2 rpos )
        {
            int x = rpos.x;
            if ( x < minX )
                minX = x;
        } );

        return minX;
    }

    int calculateY() const
    {
        int minY = 0;

        eachRoom( [&]( const Room& room, vec2 rpos )
        {
            int y = rpos.y;
            if ( y < minY )
                minY = y;
        } );

        return minY;
    }

    ivec2 calculatePos() const
    {
        int minX = 0;
        int minY = 0;

        eachRoom( [&]( const Room& room, vec2 rpos )
        {
            int x = rpos.x;
            if ( x < minX )
                minX = x;

            int y = rpos.y;
            if ( y < minY )
                minY = y;
        } );

        return { minX, minY };
    }

    int right() const
    {
        return _size.x;
    }

    int bottom() const
    {
        return _size.y;
    }

    ivec2 dimensions() const
    {
        return _size;
    }

    int left() const
    {
        return _pos.x;
    }

    int top() const
    {
        return _pos.y;
    }

    ivec2 position() const
    {
        return _pos;
    }

    ivec4 rect() const
    {
        return ivec4( position(), dimensions() );
    }

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

    const LevelTile* findTile( int x, int y ) const
    {
        return const_cast< Dungeon& >( *this ).findTile( x, y );
    }

    vec2 tilePos( const LevelTile* pTile ) const
    {
        for ( const DungeonRoom& room : rooms )
            if ( room.owns( pTile ) )
                return (vec2) room.tilePos( pTile ) + room.pos;
        assert( false );
        return {};
    }

    auto distanceEstimateFunc()
    {
        return [this]( LevelTile& start, LevelTile& goal )
        {
            return manhattan( tilePos( &start ), tilePos( &goal ) );
        };
    }

    auto tileCostFunc()
    {
        return [this]( LevelTile& tile )
        {
            return 1;
        };
    }

    auto neighborsFunc()
    {
        return [this]( LevelTile& tile )
        {
            vec2 pos = tilePos( &tile );

            static const ivec2 VECTORS[] {
                { 0, 1 }, { 1, 0 }, { 0, -1 }, { -1, 0 }
            };

            for ( ivec2 v : VECTORS )
                if ( LevelTile* pTile = findTile( pos.x + v.x, pos.y + v.y ) )
                    if ( pTile->tileType == Tile::FLOOR )
                        co_yield std::ref( *pTile );
        };
    }

    void addRoom( Room room, vec2 pos = { 0, 0 } )
    {
        int width = room.width() + pos.x;
        int height = room.height() + pos.y;
        int x = pos.x;
        int y = pos.y;

        if ( width > _size.x )
            _size.x = width;

        if ( height > _size.y )
            _size.y = height;

        if ( x < _pos.x )
            _pos.x = x;

        if ( y < _pos.y )
            _pos.y = y;

        rooms.emplace_back( move( room ), pos );
    }

    void removeRoom( Room& room )
    {
        for ( uint i = 0; i < rooms.size(); ++i )
        {
            if ( &rooms[ i ] == &room )
            {
                int width = room.width() + rooms[ i ].pos.x;
                int height = room.height() + rooms[ i ].pos.y;
                bool recalcDims = width == _size.x || height == _size.y;

                int x = rooms[ i ].pos.x;
                int y = rooms[ i ].pos.y;
                bool recalcPos = x == _pos.x || y == _pos.y;

                rooms.erase( rooms.begin() + i );

                if ( recalcDims )
                    _size = calculateDimensions();

                if ( recalcPos )
                    _pos = calculatePos();

                break;
            }
        }
    }

    template< typename Func >
    void eachRoom( Func&& func )
    {
        for ( DungeonRoom& room : rooms )
            func( (Room&) room, room.pos );
    }

    template< typename Func >
    void eachRoom( Func&& func ) const
    {
        for ( const DungeonRoom& room : rooms )
            func( (const Room&) room, room.pos );
    }

    void settleRooms()
    {
        for ( DungeonRoom& room : rooms )
            room.pos = glm::round( room.pos );
    }

    auto enumerate()
    {
        for ( DungeonRoom& room : rooms )
            co_yield pass( (Room&) room, room.pos );
    }
};
