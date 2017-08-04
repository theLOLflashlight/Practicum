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
#include "ValueTween.h"
#include "Animation.h"

#include "Util.h"
#include "Dungeon.h"
#include "random.h"
#include "Astar.h"

#include "Player.h"

#include <glm/gtx/transform.hpp>
#include <vector>
#include <array>
#include <bitset>
#include <functional>


template< typename T >
class RandomOccurrence
{
    using Value = T;

    struct Outcome
    {
        Value outcome;
        float weight;
    };

    float weight;
    std::vector< Outcome > outcomes;

public:

    Value& select()
    {
        float r = rand_real( weight );
        float t = 0;

        for ( Outcome& o : outcomes )
            if ( (t += o.weight) > r )
                return o.outcome;

        return outcomes[ 0 ].outcome;
    }
};


bool proc( double ratePerMin, uint elapsed )
{
    return chance( ratePerMin * (elapsed / 60000) );
}

template< typename Func >
auto proc( double ratePerMin, uint elapsed, Func&& func )
{
    if ( proc( ratePerMin, elapsed ) )
        return func();

    return {};
}


using DungeonComponentManager = ComponentManager< 1000,
    Position, HitPoints, Texture, Stats, Animation, Action >;

class DungeonPlayer
    : public PlayerGlue
{
    Eid eid = -1;
    DungeonComponentManager* pComps;

public:

    DungeonPlayer() = default;

    explicit DungeonPlayer( DungeonComponentManager& comps )
        : eid { comps.newEntity() }
        , pComps { &comps }
    {
        comps.attach( eid, Position {} );
        comps.attach( eid, HitPoints {} );
        comps.attach( eid, Texture {} );
        comps.attach( eid, Stats {} );
        comps.attach( eid, Animation {} );
        comps.attach( eid, Action {} );
    }

    ~DungeonPlayer()
    {
        pComps->deleteEntity( eid );
    }

    Position& position() override
    {
        return pComps->get< Position >( eid );
    }

    HitPoints& health() override
    {
        return pComps->get< HitPoints >( eid );
    }

    Texture& texture() override
    {
        return pComps->get< Texture >( eid );
    }

    Stats& stats() override
    {
        return pComps->get< Stats >( eid );
    }

    Animation& animation() override
    {
        return pComps->get< Animation >( eid );
    }

    Action& action() override
    {
        return pComps->get< Action >( eid );
    }

    Eid id() override
    {
        return eid;
    }
};

constexpr Stats WARRIOR_STATS { 7, 4, 2, 3, Obstruction::GROUND };
constexpr Stats ROGUE_STATS { 5, 3, 3, 5, Obstruction::GROUND };
constexpr Stats MAGE_STATS { 5, 3, 5, 3, Obstruction::GROUND };
constexpr Stats PALADIN_STATS { 6, 4, 3, 3, Obstruction::GROUND };

using PositionTween = TweenEntry< Position >;

const Texture WARRIOR_TEX {
    WARRIOR, Rect { 0, 0, 16, 16 },
    TEXTURE_SIZE[ WARRIOR ]
};

const Texture ROGUE_TEX {
    ROGUE, Rect { 0, 0, 16, 16 },
    TEXTURE_SIZE[ ROGUE ]
};

const Texture MAGE_TEX {
    MAGE, Rect { 0, 0, 16, 16 },
    TEXTURE_SIZE[ MAGE ]
};

const Texture PALADIN_TEX {
    PALADIN, Rect { 0, 0, 16, 16 },
    TEXTURE_SIZE[ PALADIN ]
};

const Texture BEHOLDER_TEX {
    ELEMENTAL, Rect { 2 * 16, 5 * 16, 16, 16 },
    TEXTURE_SIZE[ ELEMENTAL ]
};

class DungeonScene
    : public Scene
    , protected RenderContext
    , protected InputReceiver
    , protected AudioEngine
    , protected DungeonComponentManager
{
public:
    
    static constexpr float RENDER_SCALE = 3;
    static constexpr float TILE_SIZE = 16;
    static constexpr float CORNER_SIZE = 4;

    static constexpr LevelTile::Floor FLOOR_TILE = LevelTile::Floor::TILE3;
    static constexpr LevelTile::Wall WALL_TILE = LevelTile::Wall::BRICK3;
    static constexpr LevelTile::Pit PIT_TILE = LevelTile::Pit::WATER1;

protected:

    Dungeon dungeon;

    //DungeonPlayer player;

    Eid playerId;

    int    currCharacterIndex = 0;
    std::vector< Eid > characters;

public:

    explicit DungeonScene( SDL_Window* pWindow )
        : Scene( pWindow )
        , playerId { newEntity() }
    {
        attach( playerId, Position {} );
        attach( playerId, HitPoints {} );
        attach( playerId, Texture {} );
        attach( playerId, Stats {} );
        attach( playerId, Animation {} );
        attach( playerId, Action {} );
    }

protected:

    void nextCharacter()
    {
        ++currCharacterIndex %= characters.size();
    }

    Eid currentCharacter()
    {
        return characters[ currCharacterIndex ];
        /*return currCharacterIndex < characters.size()
            ? characters[ currCharacterIndex ] : -1;*/
    }

    std::vector< Eid > entitiesOnTile( LevelTile* pTile )
    {
        std::vector< Eid > eids;
        Position tilePos = glm::round( dungeon.tilePos( pTile ) );

        invokeSystem( [&]( Eid eid, Position pos ) {
            if ( glm::round( pos ) == flip_y( tilePos * TILE_SIZE ) )
                eids.push_back( eid );
        } );

        return eids;
    }

    void moveEntity( Eid eid, LevelTile* pTile )
    {
        if ( pTile == nullptr )
            return;

        bool blockMove = pTile->tileType != Tile::FLOOR;

        vec2 tilePos = dungeon.tilePos( pTile );

        if ( hasAttached< Stats >( eid ) )
        {
            Stats& nttStats = get< Stats >( eid );

            for ( Eid tileNtt : entitiesOnTile( pTile ) )
            {
                invokeProcess( tileNtt, [&]( Position& pos, Stats& stats )
                {
                    if ( (nttStats.blocks & stats.blocks) != 0 )
                    {
                        blockMove = true;
                        //basicAttack( ntt, tileNtt );
                    }
                } );

                if ( blockMove ) break;
                // @TODO: handle aoe basic attacks?
            }
        }

        if ( !blockMove && hasAttached< Position >( eid ) )
        {
            posTweens.push_back( PositionTween( eid,
                flip_y( tilePos * TILE_SIZE ), prevTicks, 200 ) );
        }
    }

    void movePlayer( vec2 delta )
    {
        auto& playerPos = get< Position >( playerId );
        auto sum = (playerPos + delta) / TILE_SIZE;
        changePlayerDir( delta );

        if ( LevelTile* pTile = dungeon.findTile( sum.x, -sum.y ) )
        {
            bool blockMove = pTile->tileType != Tile::FLOOR;

            invokeSystem( [&]( Eid eid, Position pos, Stats& stats )
            {
                if ( pos / TILE_SIZE == sum
                    && (get< Stats >( playerId ).blocks & stats.blocks) != 0 )
                {
                    blockMove = true;
                    if ( hasAttached< HitPoints >( eid ) )
                        basicAttack( playerId, eid );
                }
            } );

            if ( !blockMove )
            {
                //playerPos += delta;
                posTweens.push_back( PositionTween(
                    playerId,
                    playerPos + delta,
                    prevTicks, 200 ) );

                Animation& anim = get< Animation >( playerId );
                if ( anim.updateFn == nullptr )
                    anim = { prevTicks, 400, [&]( float frac ) {
                        get< Texture >( playerId ).spriteView.x = floor( frac * 4 ) * TILE_SIZE;
                    } };
            }
            get< Action >( playerId ) = [] { return ActionResult( false ); };
        }
        // Movement not allowed.
    }

    void changePlayerDir( vec2 delta )
    {
        int dir = 0;

        if ( delta.x == 0 )
        {
            if ( delta.y > 0 )
                dir = 3;
            else if ( delta.y < 0 )
                dir = 0;
        }
        else if ( delta.y == 0 )
        {
            if ( delta.x > 0 )
                dir = 2;
            else if ( delta.x < 0 )
                dir = 1;
        }

        get< Texture >( playerId ).spriteView.y = dir * TILE_SIZE;
    }

    void basicAttack( Eid attacker, Eid defender )
    {
        #define CHECK match_signature< Position, Stats, HitPoints >

        if ( CHECK( attacker ) && CHECK( defender ) )
        {
            get< HitPoints >( defender ) -= get< Stats >( attacker ).attack;
        }

        #undef CHECK
    }

    struct BlackSquare { vec2 pos; vec2 size; };
    std::vector< BlackSquare > blackSquares;

    struct FloorCorner { vec2 pos; vec2 off; };
    std::vector< FloorCorner > floorCorners;

    std::vector< PositionTween > posTweens;

    std::vector< EntityId > enemies;

    void initExtras()
    {
        blackSquares.clear();
        floorCorners.clear();

        dungeon.eachRoom( [&]( Room& room, vec2 rpos )
        {
            room.eachTile( [&]( LevelTile& tile, vec2 tpos )
            {
                vec2 pos = flip_y( rpos + tpos ) * TILE_SIZE;

                switch ( tile.tileType )
                {
                case Tile::FLOOR :
                {
                    // Checks that a floor tile should draw a specific corner.
                    #define CHECK_TILE( A, B ) \
                    (!tile[ A##_##B ] && tile[ A ] && tile[ B ])

                    static constexpr float CORNER_OFF = TILE_SIZE - CORNER_SIZE;
                    vec2 off = vec2( tile.offset + ivec2( 4, 0 ) ) * TILE_SIZE;

                    if ( CHECK_TILE( NORTH, EAST ) )
                        floorCorners.push_back( {
                            pos + vec2( 0, 0 ),
                            off + vec2( CORNER_OFF, 0 )
                    } );
                    if ( CHECK_TILE( NORTH, WEST ) )
                        floorCorners.push_back( {
                            pos + vec2( CORNER_SIZE - TILE_SIZE, 0 ),
                            off + vec2( 0, 0 )
                    } );
                    if ( CHECK_TILE( SOUTH, EAST ) )
                        floorCorners.push_back( {
                            pos + vec2( 0, CORNER_SIZE - TILE_SIZE ),
                            off + vec2( CORNER_OFF )
                    } );
                    if ( CHECK_TILE( SOUTH, WEST ) )
                        floorCorners.push_back( {
                            pos + vec2( CORNER_SIZE - TILE_SIZE ),
                            off + vec2( 0, CORNER_OFF )
                    } );
                    #undef CHECK_TILE
                    break;
                }
                case Tile::WALL :
                {
                    float edgeOff = TILE_SIZE / 8;
                    vec2 halfTile( TILE_SIZE * 0.5, TILE_SIZE - edgeOff );
                    vec2 edgeTile( TILE_SIZE * 0.5, edgeOff );

                    if ( tile[ SOUTH ] )
                    {
                        if ( tile[ EAST ] && tile[ SOUTH_EAST ] )
                            blackSquares.push_back( {
                                pos + vec2( 0, -edgeOff ),
                                halfTile
                        } );
                        if ( tile[ WEST ] && tile[ SOUTH_WEST ] )
                            blackSquares.push_back( {
                                pos + vec2( -TILE_SIZE * 0.5, -edgeOff ),
                                halfTile
                        } );
                    }
                    if ( tile[ NORTH ] )
                    {
                        if ( tile[ EAST ] && tile[ NORTH_EAST ] )
                            blackSquares.push_back( {
                                pos + vec2( 0, 0 ),
                                edgeTile
                        } );
                        if ( tile[ WEST ] && tile[ NORTH_WEST ] )
                            blackSquares.push_back( {
                                pos + vec2( -TILE_SIZE * 0.5, 0 ),
                                edgeTile
                        } );
                    }
                    break;
                }
                default:
                    break;
                }
            } );
        } );
    }

    void updateConnections()
    {
        dungeon.eachRoom( [&]( Room& room, vec2 rpos )
        {
            room.eachTile( [&]( LevelTile& tile, vec2 tpos )
            {
                ivec2 pos = rpos + tpos;
                LevelTile nullTile( Tile::NONE );

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

                tile.updateConnections( adjs );
                #undef TILE
            } );
        } );
    }

    void eliminateSingleWalls()
    {
        dungeon.eachRoom( [&]( Room& room, vec2 rpos )
        {
            room.eachTile( [&]( LevelTile& tile, vec2 tpos )
            {
                if ( tile.tileType == Tile::WALL )
                    if ( tile.noneConnects( { NORTH, EAST, SOUTH, WEST } ) )
                        tile = FLOOR_TILE;

                //if ( tile.tileType == Tile::FLOOR )
                //    if ( tile.noneConnects( { NORTH, EAST, SOUTH, WEST } ) )
                //        tile = LevelTile::Wall::BRICK3;
            } );
        } );

        updateConnections();
    }

    void useTexture( TextureId texture )
    {
        useTextureUnit( texture );
        useSize( TEXTURE_SIZE[ texture ] );
    }

    Eid spawnEnemy( Position pos, Stats stats, Texture tex )
    {
        Eid eid = newEntity();
        attach( eid, pos );
        attach( eid, stats );
        attach( eid, tex );
        attach( eid, stats.maxHealth );
        attach( eid, Action { [&, eid]
        {
            if ( chance( 0.02 ) )
            {
                static const vec2 VECTORS[]
                {
                    { 1, 0 },{ -1, 0 },{ 0, 1 },{ 0, -1 }
                };

                vec2 delta = VECTORS[ rand_int( 3 ) ];
                vec2 pos = get< Position >( eid ) / TILE_SIZE + delta;
                //vec2 pos = randTilePos( Tile::FLOOR ) / TILE_SIZE;
                moveEntity( eid, dungeon.findTile( pos.x, -pos.y ) );

                return ActionResult( true );
            }
            return ActionResult( false );
        } } );
        return eid;
    }

    Position randTilePos( Tile tile ) const
    {
        ivec4 d = dungeon.rect();
        int x, y;
        const LevelTile* pTile;

        do {
            x = rand_int( d.x, d.z );
            y = rand_int( d.y, d.w );
            pTile = dungeon.findTile( x, y );
        }
        while ( !pTile || pTile->tileType != tile );

        return Position { x, -y } * TILE_SIZE;
    }

public:

    void init( uint ticks ) override
    {
        Scene::init( ticks );

        // Set up camera.
        camPos = { -TILE_SIZE, 0 };
        camArea = vec2( width, height ) / RENDER_SCALE;

        Room room( 7, 7 );

        auto scrambleRoom = [&] {
            room.eachTile( [&]( LevelTile& tile, ivec2 pos )
            {
                if ( chance( 0.5 ) )
                {
                    tile = FLOOR_TILE;
                }
                else
                {
                    //tile = PIT_TILE;
                    tile = WALL_TILE;
                }
            } );
        };

        scrambleRoom();
        dungeon.addRoom( room, { 0, 0 } );
        dungeon.addRoom( room, { 7, 7 } );
        scrambleRoom();
        dungeon.addRoom( room, { 7, 0 } );
        dungeon.addRoom( room, { 14, 7 } );
        scrambleRoom();
        dungeon.addRoom( room, { 0, 7 } );
        dungeon.addRoom( room, { 14, 0 } );

        for ( int x = 0; x < room.width() * 3; ++x )
        {
            dungeon.getTile( x, 0 ) = WALL_TILE;
            //dungeon.getTile( x, room.bottom() * 2 - 1 ) = WALL_TILE;
        }

        for ( int y = 0; y < room.height() * 2; ++y )
        {
            dungeon.getTile( 0, y ) = WALL_TILE;
            dungeon.getTile( room.width() * 3 - 1, y ) = WALL_TILE;
        }

        updateConnections();
        eliminateSingleWalls();
        initExtras();

        invokeProcess( playerId,
            [&]( Stats& stats, HitPoints& hp, Position& pos, Texture& tex )
        {
            stats = WARRIOR_STATS;
            hp = WARRIOR_STATS.maxHealth;
            pos = randTilePos( Tile::FLOOR );
            tex = WARRIOR_TEX;
        } );

        characters.push_back( playerId );

        static constexpr Stats ENEMY_STATS { 10, 3, 3, 2, Obstruction::GROUND };

        for ( int i = 0; i < 5; ++i )
            characters.push_back( spawnEnemy(
                randTilePos( Tile::FLOOR ), ENEMY_STATS, BEHOLDER_TEX ) );

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

        GLuint tex9 = load_texture( FONT1, "Textures/Fonts/Expire.png" );

        GLuint tex10 = load_texture( ELEMENTAL, "Textures/Enemies/Elemental0.png" );
    }

    void resume( uint ticks ) override
    {
        Scene::resume( ticks );
        useClear( vec4( vec3( 20, 12, 28 ) / 255f, 1 ) );
    }

    void pause( uint ticks ) override
    {
        Scene::pause( ticks );
    }

    void exit( uint ticks ) override
    {
        Scene::exit( ticks );
    }

    std::bitset< 4 > movementBuffer;

    void update( uint ticks ) override
    {
        Scene::update( ticks );

        // Player arrow key movement.
        auto keys = wereKeysPressed( SDLK_RIGHT, SDLK_LEFT, SDLK_DOWN, SDLK_UP );

        // Only allow player input if not tweening.
        if ( none_of( posTweens,
                     [&]( PositionTween& tween ) {
                         return playerId == tween.eid;
                     } ) )
        {
            if ( movementBuffer.any() )
            {
                keys = movementBuffer;
                movementBuffer.reset();
            }

            get< Texture >( playerId ).spriteView.x = 0;

            if ( keys[ 0 ] )
                get< Action >( playerId ) = [&] {
                    movePlayer( vec2( TILE_SIZE, 0 ) );
                    return ActionResult( true );
                };
            if ( keys[ 1 ] )
                get< Action >( playerId ) = [&] {
                    movePlayer( vec2( -TILE_SIZE, 0 ) );
                    return ActionResult( true );
                };
            if ( keys[ 2 ] )
                get< Action >( playerId ) = [&] {
                    movePlayer( vec2( 0, -TILE_SIZE ) );
                    return ActionResult( true );
                };
            if ( keys[ 3 ] )
                get< Action >( playerId ) = [&] {
                    movePlayer( vec2( 0, TILE_SIZE ) );
                    return ActionResult( true );
                };

            if ( keys.none() )
                get< Animation >( playerId ).updateFn = nullptr;
        }
        else
        {
            if ( keys.any() )
                movementBuffer = keys;
        }

        invokeProcess( currentCharacter(), [&]( Action& action )
        {
            while ( 1 )
            {
                ActionResult result = action.perform();

                if ( !result.succeeded )
                    return;

                if ( !!result.backupAction )
                    break;

                action = move( result.backupAction );
            }

            nextCharacter();
        } );

        // Update animations.
        invokeSystem( [&]( Eid, Animation& anim ) {
            anim.update( ticks );
        } );

        // Perform tweens.
        for ( auto& tween : posTweens )
        {
            tween( ticks, get< Position >( tween.eid ) );
        }

        // Remove expired tweeners.
        remove_elements( posTweens, [ticks]( PositionTween& e ) {
            return e.expired( ticks );
        } );

        // Handle entity death
        {
            // Can't add or remove entities while invoking a system
            // so we store the ids in a temporary array.
            std::vector< Eid > deadEntities;

            invokeSystem( [&]( Eid eid, HitPoints& hp )
            {
                if ( hp <= 0 )
                    deadEntities.push_back( eid );
            } );

            for ( Eid eid : deadEntities )
            {
                deleteEntity( eid );
                remove_elements( characters, [eid]( Eid nmeid ) {
                    return nmeid == eid;
                } );
                currCharacterIndex %= characters.size();
            }
        }

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

                vec2 pos = flip_y( rpos + tpos ) * TILE_SIZE;
                fillRect( pos, vec2( TILE_SIZE ) );
            } );
        } );

        // Black out area between walls.
        {
            useTexture( DEFAULT );
            useColor( vec4( vec3( 20, 12, 28 ) / 255f, 1 ) );
            useSprite( { 0, 0, 1, 1 } );

            for ( auto square : blackSquares )
                fillRect( square.pos, square.size );
        }
        // Draw obtuse floor corners.
        {
            useTexture( FLOOR );
            useColor( { 1, 1, 1, 1 } );

            for ( auto corner : floorCorners )
            {
                useSprite( vec4( corner.off, vec2( CORNER_SIZE ) ) );
                fillRect( corner.pos, vec2( CORNER_SIZE ) );
            }
        }

        invokeSystem( [&]( Eid eid, Position pos, Texture tex )
        {
            useTexture( (TextureId) tex.textureUnit );
            useSprite( tex.spriteView );
            useColor( tex.color );

            fillRect( pos, vec2( TILE_SIZE ) );
        } );

        // Draw healthbars
        invokeSystem( [&]( Eid eid, Position pos, HitPoints hp, Stats stats )
        {
            if ( playerId == eid )
                return;

            useTexture( DEFAULT );
            useSprite( { 0, 0, 1, 1 } );

            float hpWidth = TILE_SIZE - 2;

            vec2 hpPos = pos; hpPos.x -= 1;
            vec2 hpSize { hpWidth, 1.5 };

            useColor( vec4( 0, 0, 0, 0.7 ) );
            fillRect( hpPos, hpSize );

            float hpFrac = hp / (float) stats.maxHealth;
            hpSize.x *= hpFrac;
            hpPos.x -= hpWidth - hpSize.x;
            vec3 hpColor = hpFrac > 0.333 ? hpFrac > 0.666
                ? vec3( 0, 1, 0 ) : vec3( 1, 1, 0 ) : vec3( 1, 0, 0 );

            useColor( vec4( hpColor, 0.9 ) );
            fillRect( hpPos, hpSize );
        } );

        // Draw Player
        {
            auto& tex = get< Texture >( playerId );
            useTextureUnit( tex.textureUnit );
            useSprite( tex.spriteView );
            useSize( tex.size );
            useColor( tex.color );
            fillRect( get< Position >( playerId ), vec2( TILE_SIZE ) );
        }

        endFrame();
    }
};
