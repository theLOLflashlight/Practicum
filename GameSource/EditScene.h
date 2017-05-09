#pragma once

#include "Scene.h"
#include "glhelp.h"
#include "Entity.h"
#include "EntityId.h"
#include "Mesh.h"
#include "Texture.h"
#include "SmartTexture.h"

#include "ComponentManager.h"
#include "InputManager.h"
#include "AudioEngine.h"
#include "RenderContext.h"

#include "Util.h"
#include "Dungeon.h"
#include "random.h"

#include <glm/gtx/transform.hpp>
#include <vector>
#include <array>
#include <bitset>

class EditScene
    : public Scene
    , protected RenderContext
    , protected InputReceiver
    , protected AudioEngine
    , protected ComponentManager< 1000,
        Position, Texture >
{
private:

    Dungeon dungeon;

    std::vector< vec2 > blackSquares;

    struct FloorCorner
    {
        vec2 pos;
        vec2 off;
    };

    std::vector< FloorCorner > floorCorners;

    void initExtras()
    {
        blackSquares.clear();
        floorCorners.clear();

        dungeon.eachRoom( [&]( Room& room, vec2 rpos )
        {
            // Draw tiles.
            room.eachTile( [&]( LevelTile& tile, vec2 tpos )
            {
                vec2 pos = flip_y( rpos + tpos + vec2( 1, 0 ) ) * TILE_SIZE;

                switch ( tile.tileType )
                {
                case TileType::WALL :
                    if ( tile[ SOUTH ] )
                    {
                        if ( tile[ EAST ] && tile[ SOUTH_EAST ] )
                            blackSquares.push_back( pos + vec2( 0, -TILE_SIZE / 8 ) );

                        if ( tile[ WEST ] && tile[ SOUTH_WEST ] )
                            blackSquares.push_back( pos + vec2( -TILE_SIZE / 2, -TILE_SIZE / 8 ) );
                    }
                    break;
                case TileType::FLOOR :
                    #define TILE_CHECK( A, B ) \
                    (!tile[ A##_##B ] && tile[ A ] && tile[ B ])

                    if ( TILE_CHECK( NORTH, WEST ) )
                        floorCorners.push_back( {
                        pos + vec2( 10 - TILE_SIZE, 0 ),
                        tile.category + ivec2( 0, -3 )
                    } );

                    if ( TILE_CHECK( NORTH, EAST ) )
                        floorCorners.push_back( {
                        pos + vec2( 0, 0 ),
                        tile.category + ivec2( 0, -3 )
                    } );

                    if ( TILE_CHECK( SOUTH, WEST ) )
                        floorCorners.push_back( {
                        pos + vec2( 10 - TILE_SIZE, 10 - TILE_SIZE ),
                        tile.category + ivec2( 0, -3 )
                    } );

                    if ( TILE_CHECK( SOUTH, EAST ) )
                        floorCorners.push_back( {
                        pos + vec2( 0, 10 - TILE_SIZE ),
                        tile.category + ivec2( 0, -3 )
                    } );

                    #undef TILE_CHECK
                    break;
                default:
                    break;
                }
            } );
        } );
    }

public:

    static constexpr float TILE_SIZE = 32;

    explicit EditScene( SDL_Window* pWindow )
        : Scene( pWindow )
    {
    }

protected:

    void useTexture( TextureId texture )
    {
        useTextureUnit( texture );
        useSize( TEXTURE_SIZE[ texture ] );
    }

public:

    void init( uint ticks ) override
    {
        Scene::init( ticks );

        // Set up camera.
        camPos = { -16, -16 };
        camArea = { width, height };

        Room room( 10, 10 );
        room.eachTile( [&]( LevelTile& tile, ivec2 pos )
        {
            if ( chance( 0.6 ) )
            {
                tile = LevelTile::TILE2;
            }
            else
            {
                tile = LevelTile::BRICK2;
            }
        } );

        dungeon.addRoom( room, { 0, 0 } );
        dungeon.addRoom( room, { 10, 0 } );
        dungeon.addRoom( room, { 0, 10 } );
        dungeon.addRoom( room, { 10, 10 } );

        // Init rooms.
        dungeon.eachRoom( [&]( Room& room, vec2 rpos )
        {
            // Init tiles.
            room.eachTile( [&]( LevelTile& tile, vec2 tpos )
            {
                ivec2 pos = rpos + tpos;
                LevelTile nullTile( TileType::NONE );

                #define TILE( X, Y ) \
                    COALESCE_NULL( dungeon.findTile( X, Y ), nullTile )
                RefArray< LevelTile, 8 > adjs
                {
                    TILE( pos.x - 1, pos.y + 0 ),
                    TILE( pos.x + 0, pos.y + 1 ),
                    TILE( pos.x + 1, pos.y + 0 ),
                    TILE( pos.x + 0, pos.y - 1 ),
                    TILE( pos.x - 1, pos.y - 1 ),
                    TILE( pos.x + 1, pos.y - 1 ),
                    TILE( pos.x + 1, pos.y + 1 ),
                    TILE( pos.x - 1, pos.y + 1 ),
                };
                #undef TILE

                tile.updateConnections( adjs );
            } );
        } );

        initExtras();

        // Load textures.
        GLuint tex0 = load_texture< GLubyte[ 4 ] >( DEFAULT, 1, 1, { 0xff, 0xff, 0xff, 0xff } );
        
        GLuint tex1 = load_texture( FLOOR, "Textures/Floor.png" );
        GLuint tex2 = load_texture( WALL, "Textures/Wall.png" );
        GLuint tex3 = load_texture( PIT, "Textures/Pit1.png" );
        GLuint tex4 = load_texture( PIT, "Textures/Pit0.png" );

        GLuint tex5 = load_texture( WARRIOR, "Textures/Warrior.png" );
        GLuint tex6 = load_texture( ROGUE, "Textures/Rogue.png" );
        GLuint tex7 = load_texture( MAGE, "Textures/Mage.png" );
        GLuint tex8 = load_texture( PALADIN, "Textures/Paladin.png" );
    }

    void resume( uint ticks ) override
    {
        Scene::resume( ticks );
    }

    void pause( uint ticks ) override
    {
        Scene::pause( ticks );
    }

    void exit( uint ticks ) override
    {
        Scene::exit( ticks );
    }

    void update( uint ticks ) override
    {
        Scene::update( ticks );
    }

    void draw() override
    {
        using namespace glm;
        Scene::draw();

        beginFrame();
        useColor( { 1, 1, 1, 1 } );

        // Draw rooms.
        dungeon.eachRoom( [&]( Room& room, vec2 rpos )
        {
            // Draw tiles.
            room.eachTile( [&]( LevelTile& tile, vec2 tpos )
            {
                useTexture( tile.getTexture() );
                useSprite( tile.getSprite() );

                vec2 pos = flip_y( rpos + tpos + vec2( 1, 0 ) ) * TILE_SIZE;
                fillRect( pos, vec2( TILE_SIZE ) );
            } );
        } );

        // Black out area between walls.
        {
            useTexture( DEFAULT );
            useColor( vec4( vec3( 20, 12, 28 ) / 255f, 1 ) );
            useSprite( { 0, 0, 1, 1 } );

            for ( vec2 pos : blackSquares )
                fillRect( pos, vec2( TILE_SIZE * 0.5, TILE_SIZE ) );
        }
        // Draw obtuse floor corners.
        {
            useTexture( FLOOR );
            useColor( { 1, 1, 1, 1 } );

            for ( auto corner : floorCorners )
            {
                useSprite( vec4( corner.off * TILE_SIZE, 5, 5 ) );
                fillRect( corner.pos, vec2( 10 ) );
            }
        }

        endFrame();
    }
};