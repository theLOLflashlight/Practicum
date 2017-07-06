#pragma once

#include "SmartTexture.h"
#include "Util.h"
#include "EntityId.h"
#include "Dungeon.h"

#include <array>
#include <cmath>
#include <vector>

struct EntityPosRemap
{
    EntityId eid;
    ivec2    pos;
};


template< size_t Width, size_t Height >
class LevelFacelet
{
public:

    static constexpr int WIDTH = Width;
    static constexpr int HEIGHT = Height;

    static constexpr Tile VOID_TILE = Tile::NONE;

    using TileCol = std::array< Tile, HEIGHT >;
    using TextureCol = std::array< SmartTexture, HEIGHT >;

    std::array< TileCol, WIDTH > tiles;
    std::array< TextureCol, WIDTH > textures;

    std::array< LevelFacelet*, 8 > adjFacelets;

//private:

    std::vector< EntityPosRemap > _posRemaps;

public:

    LevelFacelet( bool wrap = false )
    {
        constexpr uint8_t MASKS[] { 0, 15, 15, 63 }; // @MagicNumbers.

        for ( int x = 0; x < WIDTH; ++x )
        {
            for ( int y = 0; y < HEIGHT; ++y )
            {
                Tile& tile = tiles[ x ][ y ];
                
                int key = (rand() % 9) < 5 ? 0 : 1;
                tile = Tile( 1 + key );

                SmartTexture& texture = textures[ x ][ y ];
                texture.mask = MASKS[ (int) tile ];
                texture.initialOffset = { 0, 0 };
            }
        }
        
        if ( wrap )
            for ( auto& ptr : adjFacelets )
                ptr = this;

        updateAdj();
    }

    /*int _rotation : 2;

    ivec2 adjIdx( int x, int y )
    {
        Point p { x, y };

        switch ( _rotation )
        {
        case 0:
            break;
        case 1:
            p.x -= WIDTH / 2;
            p.y -= HEIGHT / 2;
            std::swap( p.x, p.y );
            p.x += WIDTH / 2;
            p.y += HEIGHT / 2;
            break;
        case 2:
            p.x = WIDTH - 1 - x;
            p.y = HEIGHT - 1 - y;
            break;
        case 3:
            p.x -= WIDTH / 2;
            p.y -= HEIGHT / 2;
            std::swap( p.x, p.y );
            p.x += WIDTH / 2;
            p.y += HEIGHT / 2;
            p.x = -p.x;
            break;
        }

        return p;
    }

    Tile& getTile( int x, int y )
    {
        ivec2 i = adjIdx( x, y );
        return tiles[ i.x, i.y ];
    }

    Tile& getTexture( int x, int y )
    {
        ivec2 i = adjIdx( x, y );
        return textures[ i.x, i.y ];
    }*/

    void updateAdj()
    {
        LevelFacelet* westFacelet = adjFacelets[ WEST ];
        LevelFacelet* southFacelet = adjFacelets[ SOUTH ];
        LevelFacelet* eastFacelet = adjFacelets[ EAST ];
        LevelFacelet* northFacelet = adjFacelets[ NORTH ];

        const TileCol& _westCol = westFacelet != nullptr
                                ? westFacelet->tiles[ WIDTH - 1 ]
                                : TileCol { VOID_TILE };
        const TileCol& _eastCol = eastFacelet != nullptr
                                ? eastFacelet->tiles[ 0 ]
                                : TileCol { VOID_TILE };

        for ( int x = 0; x < WIDTH; ++x )
        {
            const TileCol& col     = tiles[ x ];
            const TileCol& westCol = x > 0 ? tiles[ x - 1 ] : _westCol;
            const TileCol& eastCol = x + 1 < WIDTH ? tiles[ x + 1 ] : _eastCol;

            for ( int y = 0; y < HEIGHT; ++y )
            {
                Tile northTile = y > 0 ? col[ y - 1 ]
                    : northFacelet != nullptr ? northFacelet->tiles[ x ][ HEIGHT - 1 ]
                        : VOID_TILE;
                Tile southTile = y + 1 < HEIGHT ? col[ y + 1 ]
                    : southFacelet != nullptr ? southFacelet->tiles[ x ][ 0 ]
                        : VOID_TILE;

                Tile tile = tiles[ x ][ y ];
                SmartTexture& texture = textures[ x ][ y ];

                texture[ WEST ]  = tile == westCol[ y ];
                texture[ SOUTH ] = tile == southTile;
                texture[ EAST ]  = tile == eastCol[ y ];
                texture[ NORTH ] = tile == northTile;
            }
        }
    }

    template< typename Func >
    void eachTile( Func&& func )
    {
        for ( int x = 0; x < WIDTH; ++x )
            for ( int y = 0; y < HEIGHT; ++y )
                std::invoke( func, tiles[x][y], textures[x][y], x, y );
    }

};