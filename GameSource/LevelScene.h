#pragma once

#include "Scene.h"
#include "glhelp.h"
#include "Drawable.h"
#include "Dictionary.h"
#include "EntityId.h"
#include "Entity.h"

#include "ComponentManager.h"
#include "SmartTexture.h"
#include "InputManager.h"
#include "AudioEngine.h"

#include "Rubiks.h"
#include "LevelFacelet.h"

#include <glm/gtx/transform.hpp>
#include <vector>
#include <array>
#include <bitset>

class LevelScene;

using SpriteInfo = glm::vec4;

class EntityView
{
public:

    EntityId eid;
    LevelScene* pScene;

    EntityView() = default;

    EntityView( EntityId eid, LevelScene* pScene )
        : eid( eid ), pScene( pScene )
    {
    }

    Entity& get();

    operator Entity&()
    {
        return get();
    }
};

class PlayerView
    : public EntityView
{
public:

    using EntityView::EntityView;

    bool moveDirection( AdjDirection dir );
};

glm::vec2 direction_offset( AdjDirection dir )
{
    switch ( dir )
    {
    case WEST: return { -1, 0 };
    case SOUTH: return { 0, +1 };
    case EAST: return { +1, 0 };
    case NORTH: return { 0, -1 };
    case NORTH_WEST:
        break;
    case NORTH_EAST:
        break;
    case SOUTH_EAST:
        break;
    case SOUTH_WEST:
        break;
    default:
        break;
    }
    return { 0, 0 };
}

struct LevelCoord
{
    ivec2 face;
    ivec2 tile;
};

struct Collectible
{
    std::function< void( Entity&, glm::vec2 ) > onCollect;
};


class LevelScene
    : public Scene
    , protected AudioEngine
    , protected InputReceiver
    , protected ComponentManager< 100,
        Drawable, glm::vec2, SpriteInfo, Collectible >
{
public:

    friend class EntityView;
    friend class PlayerView;

    Permutation rubiksCube;

    GLuint programId;
    const GLuint uMatrix, uSprite, uTexture, uColor;

    PlayerState playerState { 0, true, 400 };

    PlayerView playerEntity { "player", this };

private:

    bool _dirtyTiles;

public:

    static constexpr int FACELET_W = 7;
    static constexpr int FACELET_H = 7;

    using LevelFace = LevelFacelet< FACELET_W, FACELET_H >;

    static constexpr int FACELET_X = 3;
    static constexpr int FACELET_Y = 3;

    static constexpr int TILE_COL_COUNT = FACELET_X * FACELET_W;
    static constexpr int TILE_ROW_COUNT = FACELET_Y * FACELET_H;

    static constexpr int TILE_OFF_X = TILE_COL_COUNT / 2;
    static constexpr int TILE_OFF_Y = TILE_ROW_COUNT / 2;

    static constexpr float ENTITY_DRAW_OFFSET_X = 0;
    static constexpr float ENTITY_DRAW_OFFSET_Y = 0.2;

    static constexpr float DRAW_SCALE = 1.0 / (TILE_COL_COUNT / 2.0);

    std::array< std::array< LevelFace, FACELET_Y >, FACELET_X > facelets;

    LevelScene( SDL_Window* pWindow )
        : Scene( pWindow )

        , programId( load_shaders( "Shaders/vert2.glsl", "Shaders/frag2.glsl" ) )
        , uMatrix( glGetUniformLocation( programId, "uMatrix" ) )
        , uSprite( glGetUniformLocation( programId, "uSprite" ) )
        , uTexture( glGetUniformLocation( programId, "uTexture" ) )
        , uColor( glGetUniformLocation( programId, "uColor" ) )
    {
    }

    static glm::vec2 random_position()
    {
        return {
            (rand() % TILE_COL_COUNT) - TILE_OFF_X,
            (rand() % TILE_ROW_COUNT) - TILE_OFF_Y
        };
    }

    glm::vec2 randomPos( TileType mustBe )
    {
        glm::vec2 pos;
        do {
            pos = random_position();
        } while ( getTile( pos ) != mustBe );
        return pos;
    }

    void init( unsigned ticks ) override
    {
        Scene::init( ticks );

        GLuint tex0 = load_texture< GLubyte[ 4 ] >( 0, 1, 1, { 0xff, 0xff, 0xff, 0xff } );
        GLuint tex1 = load_texture( 1, "Textures/Warrior.png" );
        GLuint tex2 = load_texture( 2, "Textures/hexagon.png" );
        GLuint tex3 = load_texture( 3, "Textures/Floor.png" );
        GLuint tex4 = load_texture( 4, "Textures/Wall.png" );
        GLuint tex5 = load_texture( 5, "Textures/Pit0.png" );
        GLuint tex6 = load_texture( 6, "Textures/Pit1.png" );
        GLuint tex7 = load_texture( 7, "Textures/Decor0.png" );

        {
            Entity& player = newEntity( "player" );

            Drawable&& d = Drawable::make_rect( 1, 1 );
            d.texture = 1;
            //d.visible = false;

            attach( player, move( d ) );
            attach( player, randomPos( TileType::FLOOR ) );
            attach( player, glm::vec4( 0, 0, 4, 4 ) );

            //detachAll( player );
            //detach< SpriteInfo >( player );
        }
        {
            Entity& floor = newEntity( "floor" );

            Drawable&& fd = Drawable::make_rect( 1, 1 );
            fd.texture = 3;
            fd.visible = false;

            attach( floor, move( fd ) );
            attach( floor, SpriteInfo( 0, 0, 21, 39 ) );
        }
        {
            Entity& wall = newEntity( "wall" );

            Drawable&& wd = Drawable::make_rect( 1, 1 );
            wd.texture = 4;
            wd.visible = false;

            attach( wall, move( wd ) );
            attach( wall, SpriteInfo( 0, 0, 20, 51 ) );
        }
        {
            Entity& pit = newEntity( "pit" );

            Drawable&& pd = Drawable::make_rect( 1, 1 );
            pd.texture = 5;
            pd.visible = false;

            attach( pit, move( pd ) );
            attach( pit, SpriteInfo( 0, 0, 8, 32 ) );
        }

        int numItems = 4 + rand() % 5;
        for ( int i = 0; i < numItems; ++i )
        {
            Entity& item = newEntity();

            Drawable&& id = Drawable::make_rect( 1, 1 );
            id.texture = 7;

            attach( item, move( id ) );
            attach( item, randomPos( TileType::FLOOR ) );
            attach( item, SpriteInfo( 4, 3, 8, 22 ) );
            attach( item, Collectible {
                [&]( Entity& ntt, glm::vec2 pos ) {
                    printf( "Collected!\n" );
                    playSound( "Audio/item_pickup_1.wav", { pos.x, 0, 0 } );
                    deleteEntity( getId( ntt ) );
                }
            } );
        }

        for ( int x = 0; x < FACELET_X; ++x )
        for ( int y = 0; y < FACELET_Y; ++y )
        facelets[ x ][ y ].eachTile( [&]( TileType tile, SmartTexture& tex, ... )
        {
            switch ( tile )
            {
            case TileType::FLOOR:
                tex.initialOffset = { 0, 3 + 3 * char( (x + y) % 4 ) };
                break;
            case TileType::WALL:
                tex.initialOffset = { 0, 3 + 3 * char( (x + y) % 4 ) };
                break;
            case TileType::PIT:
                tex.initialOffset = { 0, 8 };
                break;
            }
        } );

        linkFaces();
        updateTileTextures();
    }

protected:

    void linkFaces()
    {
        for ( int x = 0; x < FACELET_X; ++x )
        {
            for ( int y = 0; y < FACELET_Y; ++y )
            {
                facelets[ x ][ y ].adjFacelets[ WEST ] = x > 0
                    ? &facelets[ x - 1 ][ y ] : nullptr;

                facelets[ x ][ y ].adjFacelets[ SOUTH ] = y + 1 < FACELET_Y
                    ? &facelets[ x ][ y + 1 ] : nullptr;

                facelets[ x ][ y ].adjFacelets[ EAST ] = x + 1 < FACELET_X
                    ? &facelets[ x + 1 ][ y ] : nullptr;

                facelets[ x ][ y ].adjFacelets[ NORTH ] = y > 0
                    ? &facelets[ x ][ y - 1 ] : nullptr;
            }
        }
    }

    void updateTileTextures()
    {
        for ( auto& col : facelets )
            for ( LevelFace& face : col )
                face.updateAdj();
    }

public:

    LevelCoord levelCoords( int x, int y )
    {
        int X = x / FACELET_W;
        int Y = y / FACELET_H;

        x += FACELET_W * 10;
        y += FACELET_H * 10;

        x %= FACELET_W;
        y %= FACELET_H;

        return { { X, Y }, { x, y } };
    }
    
    LevelCoord levelCoords( glm::vec2 pos )
    {
        return levelCoords( (int) pos.x + TILE_OFF_X, (int) pos.y + TILE_OFF_Y );
    }


    TileType* findTile( int x, int y )
    {
        if ( x < 0 || y < 0 || x >= TILE_COL_COUNT || y >= TILE_ROW_COUNT )
            return nullptr;

        auto pos = levelCoords( x, y );

        return &facelets[ pos.face.x ][ pos.face.y ]
                  .tiles[ pos.tile.x ][ pos.tile.y ];
    }

    TileType getTile( int x, int y )
    {
        TileType* pTile = findTile( x, y );
        return pTile ? *pTile : TileType::NONE;
    }

    TileType getTile( glm::vec2 pos )
    {
        return getTile( (int) pos.x + TILE_OFF_X, (int) pos.y + TILE_OFF_Y );
    }

    void setTile( int x, int y, TileType type )
    {
        if ( TileType* pTile = findTile( x, y ) )
        {
            _dirtyTiles = *pTile != type;
            *pTile = type;
        }
    }

    void setTile( glm::vec2 pos, TileType type )
    {
        setTile( (int) pos.x + TILE_OFF_X, (int) pos.y + TILE_OFF_Y, type );
    }

    void resume( unsigned ticks ) override
    {
        Scene::resume( ticks );
    }


private:

    int _victorySoundId = -1;
    Delay _victoryDelay { 1000 };
    Delay _exitDelay { 12500 };

public:

    void update( unsigned ticks ) override
    {
        Scene::update( ticks );

        _victoryDelay.triggerIf(
            ticks, count< Collectible >() == 0 && _victorySoundId == -1,
            [&] {
                _victorySoundId = playSound( "Audio/victory_sound.wav" );
                printf( "You win!\n" );
            } );

        _exitDelay.triggerIf( ticks, _victorySoundId != -1, SDL_Quit );

        doPermutations();

        Entity* player = findEntity( "player" );

        if ( player && match_signature< glm::vec2 >( *player ) )
        {
            auto playerPos = get< glm::vec2 >( *player );

            invokeSystem(
                [&]( Entity& ntt, glm::vec2 pos, Collectible& item ) {
                    if ( pos == playerPos )
                        item.onCollect( ntt, pos );
                } );
        }

        // @Verbose. This should be much nicer.
        if ( player && match_signature< glm::vec2, SpriteInfo >( *player ) )
        {
            auto& pos = get< glm::vec2 >( *player );
            auto& tex = get< SpriteInfo >( *player );

            playerState.delay.triggerIf( ticks, playerState.isMoving, [&] {
                if ( tex.x >= tex.z - 1 )
                    tex.x = 0;
                else
                    tex.x++;
            } );

            if ( wasKeyPressed( SDLK_SPACE ) )
            {
                constexpr AdjDirection DIRECTIONS[] {
                    SOUTH, WEST, EAST, NORTH
                };

                auto tilePos = pos + direction_offset( DIRECTIONS[ (int) tex.y ] );

                TileType tileType = TileType::FLOOR;
                if ( getTile( tilePos ) == TileType::FLOOR )
                    tileType = TileType::WALL;

                setTile( tilePos, tileType );
                playSound( "Audio/click.wav", glm::vec3( tilePos, 0 ) / 5.0f );
            }

            if ( wasKeyPressed( SDLK_UP ) )
            {
                if ( tex.y != 0 )
                    playerEntity.moveDirection( NORTH );
                tex.y = 3;
            }

            if ( wasKeyPressed( SDLK_RIGHT ) )
            {
                if ( tex.y != 1 )
                    playerEntity.moveDirection( EAST );
                tex.y = 2;
            }

            if ( wasKeyPressed( SDLK_DOWN ) )
            {
                if ( tex.y != 3 )
                    playerEntity.moveDirection( SOUTH );
                tex.y = 0;
            }

            if ( wasKeyPressed( SDLK_LEFT ) )
            {
                if ( tex.y != 2 )
                    playerEntity.moveDirection( WEST );
                tex.y = 1;
            }
        }

        if ( _dirtyTiles )
        {
            linkFaces();
            updateTileTextures();
            _dirtyTiles = false;
        }
    }

    void draw() override
    {
        using namespace glm;
        Scene::draw();

        float aspect = width / (float) height;
        const mat4 base = scale( {}, vec3( DRAW_SCALE, DRAW_SCALE * aspect, 1 ) );

        glUseProgram( programId );

        std::reference_wrapper< Drawable > tileDrws[] {
            get< Drawable >( getEntity( "floor" ) ),
            get< Drawable >( getEntity( "wall" ) ),
            get< Drawable >( getEntity( "pit" ) ),
        };

        const SpriteInfo tileOffs[] {
            get< SpriteInfo >( getEntity( "floor" ) ),
            get< SpriteInfo >( getEntity( "wall" ) ),
            get< SpriteInfo >( getEntity( "pit" ) ),
        };

        for ( int x = 0; x < FACELET_X; ++x )
        for ( int y = 0; y < FACELET_Y; ++y )
            facelets[ x ][ y ].eachTile( [&]( TileType tile, SmartTexture& texture, int x2, int y2 )
            {
                static const TextureOffset* const OFFSETS[] {
                    FLOOR_OFFSETS, WALL_OFFSETS, PIT_OFFSETS
                };

                if ( tile == TileType::NONE )
                    return;

                const int tileInt = (int) tile - 1;
                Drawable& tileDrw = tileDrws[ tileInt ];

                glm::mat4 mtx = translate( base, vec3(
                    x2 - TILE_OFF_X + (LevelFace::WIDTH * x),
                    -y2 + TILE_OFF_Y - (LevelFace::HEIGHT * y),
                    0 ) );
                //mtx = rotate( mtx, ntt.rotation, vec3( 0, 0, 1 ) );

                SpriteInfo texAdjust = tileOffs[ tileInt ];

                TextureOffset off = texture.textureOffset( OFFSETS[ tileInt ] );
                texAdjust.x = texture.initialOffset.x + off.x;
                texAdjust.y = texture.initialOffset.y + off.y;

                glUniform( uMatrix, mtx );
                glUniform( uSprite, texAdjust );
                glUniform( uTexture, tileDrw.texture );
                glUniform( uColor, tileDrw.color );

                glBindVertexArray( tileDrw.vertexArray );
                glDrawArrays( tileDrw.mode, 0, tileDrw.count );

            } );

        invokeSystem( [&]( Entity& ntt, Drawable& d )
        {
            if ( !d.visible )
                return;

            glm::mat4 mtx
                = match_signature< vec2 >( ntt )
                ? translate( base, vec3(
                    +get< vec2 >( ntt ).x + ENTITY_DRAW_OFFSET_X,
                    -get< vec2 >( ntt ).y + ENTITY_DRAW_OFFSET_Y,
                    0 ) )
                : base;
            //mtx = rotate( mtx, ntt.rotation, vec3( 0, 0, 1 ) );

            SpriteInfo spriteInfo
                = match_signature< SpriteInfo >( ntt )
                ? get< SpriteInfo >( ntt )
                : SpriteInfo( 0, 0, 1, 1 );

            glUniform( uMatrix, mtx );
            glUniform( uSprite, spriteInfo );
            glUniform( uTexture, d.texture );
            glUniform( uColor, d.color );

            glBindVertexArray( d.vertexArray );
            glDrawArrays( d.mode, 0, d.count );
        } );
    }

    void doPermutations()
    {
        using glm::vec2;

        const bool requiresRemap = wereKeysPressed(
            SDLK_1, SDLK_2, SDLK_3, SDLK_4, SDLK_5, SDLK_6 ).any();

        if ( requiresRemap )
        {
            for ( auto entry : getEntities() )
            {
                if ( match_signature< vec2 >( entry.value ) )
                {
                    vec2 worldPos = get< vec2 >( entry.value );

                    auto coords = levelCoords( worldPos );

                    facelets[ coords.face.x ][ coords.face.y ]
                        ._posRemaps.push_back( { entry.key, coords.tile } );
                }
            }
        }

        if ( wasKeyPressed( SDLK_1 ) )
        {
            rotate(
                facelets[ 0 ][ 0 ],
                facelets[ 1 ][ 1 ],
                facelets[ 2 ][ 2 ] );
            _dirtyTiles = true;
        }
        if ( wasKeyPressed( SDLK_2 ) )
        {
            unrotate(
                facelets[ 2 ][ 0 ],
                facelets[ 1 ][ 1 ],
                facelets[ 0 ][ 2 ] );
            _dirtyTiles = true;
        }
        if ( wasKeyPressed( SDLK_3 ) )
        {
            rotate(
                facelets[ 1 ][ 0 ],
                facelets[ 2 ][ 1 ],
                facelets[ 1 ][ 2 ],
                facelets[ 0 ][ 1 ] );
            _dirtyTiles = true;
        }
        if ( wasKeyPressed( SDLK_4 ) )
        {
            rotate(
                facelets[ 0 ][ 0 ],
                facelets[ 1 ][ 0 ],
                facelets[ 2 ][ 0 ],
                facelets[ 2 ][ 1 ],
                facelets[ 2 ][ 2 ],
                facelets[ 1 ][ 2 ],
                facelets[ 0 ][ 2 ],
                facelets[ 0 ][ 1 ] );
            _dirtyTiles = true;
        }
        if ( wasKeyPressed( SDLK_5 ) )
        {
            rotate(
                facelets[ 1 ][ 0 ],
                facelets[ 1 ][ 1 ],
                facelets[ 1 ][ 2 ] );
            _dirtyTiles = true;
        }
        if ( wasKeyPressed( SDLK_6 ) )
        {
            rotate(
                facelets[ 0 ][ 1 ],
                facelets[ 1 ][ 1 ],
                facelets[ 2 ][ 1 ] );
            _dirtyTiles = true;
        }

        if ( requiresRemap )
        {
            for ( int x = 0; x < FACELET_X; ++x )
            {
                for ( int y = 0; y < FACELET_Y; ++y )
                {
                    for ( auto remap : facelets[ x ][ y ]._posRemaps )
                    {
                        vec2& worldPos = get< vec2 >( getEntity( remap.eid ) );

                        worldPos.x = x * FACELET_W + remap.pos.x;
                        worldPos.y = y * FACELET_H + remap.pos.y;

                        worldPos -= vec2 { TILE_OFF_X, TILE_OFF_Y };
                    }
                    facelets[ x ][ y ]._posRemaps.clear();
                }
            }
        }
    }
    
};


inline Entity& EntityView::get()
{
    return pScene->getEntity( eid );
}

bool PlayerView::moveDirection( AdjDirection dir )
{
    Entity& player = get();

    auto& pos = pScene->get< glm::vec2 >( player );
    auto off = pos + direction_offset( dir );

    TileType tile = pScene->getTile(
        (int) off.x + LevelScene::TILE_OFF_X,
        (int) off.y + LevelScene::TILE_OFF_Y );

    if ( tile == TileType::FLOOR )
    {
        pos = off;
        return true;
    }

    return false;
}

//Entity* EntityView::operator ()( LevelScene* scene )
//{
//    return scene->findEntity( eid );
//}
//
//Entity& EntityView::operator ()( LevelScene& scene )
//{
//    return scene.getEntity( eid );
//}


