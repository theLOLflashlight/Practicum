#pragma once

#include "Scene.h"
#include "glhelp.h"
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
#include "Delay.h"
#include "Behavior.h"
#include "Npc.h"

#include <glm/gtx/transform.hpp>
#include <vector>
#include <array>
#include <bitset>

#define FOR_COMPONENT( TYPE, NAME ) for ( TYPE& NAME : components< TYPE >() )

enum DungeonEntityFlags
{
    IS_DEAD,
    IS_ACTIVE,
    _last
};

using Behavior = VariadicBehavior<
    IdleBehavior, WanderBehavior, PathBehavior >;

using DungeonComponentManager = ComponentManager< 1000, DungeonEntityFlags,
    Position, HitPoints, Texture, Stats, Animation, Action, Delay, Behavior >;

struct Entity
{
    const Eid eid;
};

struct HpEntity : Entity
{
    HitPoints& hp;

    HpEntity( tuple< Eid, HitPoints& > src )
        : Entity { std::get< 0 >( src ) }
        , hp { std::get< 1 >( src ) }
    {
    }
};

struct BlockerEntity : Entity
{
    Position& pos;
    Stats&    stats;

    BlockerEntity( tuple< Eid, Position&, Stats& > src )
        : Entity { std::get< 0 >( src ) }
        , pos { std::get< 1 >( src ) }
        , stats { std::get< 2 >( src ) }
    {
    }
};

using PositionTween = TweenEntry< Position >;

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

    static constexpr unsigned ENEMY_ACTION_DELAY = 200;

    static constexpr LevelTile::Floor FLOOR_TILE = LevelTile::Floor::TILE3;
    static constexpr LevelTile::Wall WALL_TILE = LevelTile::Wall::BRICK3;
    static constexpr LevelTile::Pit PIT_TILE = LevelTile::Pit::WATER1;

    struct AI
    {
        DungeonScene& ds;
        Eid           eid;

        AI( DungeonScene* pScene, Eid eid )
            : ds { *pScene }
            , eid { eid }
        {
            assert( pScene != nullptr );
        }

        template< typename T >
        Action operator ()( T& )
        {
            return [] { return ActionResult( true ); };
        }

        Action operator ()( WanderBehavior& bhvr );

        Action operator ()( PathBehavior& bhvr );
    };

public:

    Dungeon dungeon;
    Eid     playerId;

protected:

    struct BlackSquare { vec2 pos; vec2 size; };
    std::vector< BlackSquare > blackSquares;

    struct FloorCorner { vec2 pos; vec2 off; };
    std::vector< FloorCorner > floorCorners;

    std::vector< PositionTween > posTweens;

private:

    int currCharacterIndex = 0;
    std::vector< Eid > characters;
    std::bitset< 4 > movementBuffer;

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

    generator< Eid > entitiesOnTile( LevelTile* pTile )
    {
        Position tilePos = glm::round( dungeon.tilePos( pTile ) );

#if _MSC_VER < 1911
        for ( auto ntt : subset< Position >() ) {
            auto eid = std::get< 0 >( ntt );
            auto pos = std::get< 1 >( ntt );
#else
        for ( auto[ eid, pos ] : entities< Position >() ) {
#endif
            if ( round( pos ) == flip_y( tilePos * TILE_SIZE ) )
                co_yield eid;
        }
    }

    ActionResult moveEntity( Eid eid, LevelTile* pTile )
    {
        if ( pTile == nullptr || pTile->tileType != Tile::FLOOR
            || !hasAttached< Position >( eid ) )
            return false;

        if ( hasAttached< Stats >( eid ) )
            for ( Eid tileEid : entitiesOnTile( pTile ) )
                if ( auto ntt = entity< Position, Stats >( tileEid ) )
                    if ( overlaps( get< Stats >( eid ), std::get< 1 >( *ntt ) ) )
                        return [=] { return basicAttack( eid, tileEid ); };

        vec2 tilePos = dungeon.tilePos( pTile );
        posTweens.push_back( PositionTween( eid,
            flip_y( tilePos * TILE_SIZE ), prevTicks, ENEMY_ACTION_DELAY ) );

        return true;
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
                    basicAttack( playerId, eid );
                }
            } );

            if ( !blockMove )
            {
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

    ActionResult basicAttack( Eid attacker, Eid defender )
    {
        #define CHECK matches< Position, Stats, HitPoints >

        if ( CHECK( attacker ) && CHECK( defender ) )
        {
            printf( "E.%i attacking E.%i\n", attacker, defender );
            get< HitPoints >( defender ) -= get< Stats >( attacker ).attack;
            return true;
        }
        return false;

        #undef CHECK
    }

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

    Eid spawnEnemy( Position pos, Stats stats, Texture tex, Behavior bhvr )
    {
        Eid eid = newEntity( pos, stats, tex, stats.maxHealth, bhvr );
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

    void start( uint ticks ) override
    {
        Scene::start( ticks );

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

        for ( int i = 0; i < 4; ++i )
            characters.push_back( spawnEnemy(
                randTilePos( Tile::FLOOR ),
                ENEMY_STATS, BEHOLDER_TEX,
                WanderBehavior( rand_int( 3 ) )
                ) );

        characters.push_back( spawnEnemy(
            randTilePos( Tile::FLOOR ),
            ENEMY_STATS, SAURON_TEX,
            PathBehavior( playerId )
        ) );
    }

    void resume( uint ticks ) override
    {
        Scene::resume( ticks );
        useClear( vec4( vec3( 20, 12, 28 ) / 255f, 1 ) );

        // Load textures. @TODO: improve performance.
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

    void pause( uint ticks ) override
    {
        Scene::pause( ticks );
    }

    void stop( uint ticks ) override
    {
        Scene::stop( ticks );
    }

private:

    void inputSystem( uint ticks )
    {
        if ( !exists( playerId ) )
            return;

        // Player arrow key movement.
        auto keys = wereKeysPressed( SDLK_RIGHT, SDLK_LEFT, SDLK_DOWN, SDLK_UP );

        bool isPlayerTweening = none_of(
            posTweens, [&]( PositionTween& tween ) {
                return playerId == tween.eid;
            } );

        // Only allow player input if not tweening.
        if ( isPlayerTweening )
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
    }

    void behaviorSystem( uint ticks )
    {
        Eid eid = currentCharacter();

        // Apply behaviors.
        if ( hasAttached< Behavior >( eid ) )
            attach( eid, visit( AI { this, eid }, get< Behavior >( eid ) ) );

        // Perform entity actions.
        if ( hasAttached< Action >( eid ) )
        {
            for ( Action& action = get< Action >( eid ); ; )
            {
                ActionResult result = perform( action );
                
                if ( result.backupAction )
                {
                    action = move( result.backupAction );
                    continue;
                }

                if ( !result.succeeded )
                    return;
                if ( !result.backupAction )
                    break;
            }
        }
        nextCharacter();
    }

    void deathSystem( uint ticks )
    {
        // Handle entity death
        for ( HpEntity ntt : entities< HitPoints >() )
            if ( ntt.hp <= 0 )
                flag< IS_DEAD >( ntt.eid ) = true;

        remove_elements( posTweens, [&, ticks]( PositionTween& tween ) {
            return tween.expired( ticks ) || flag< IS_DEAD >( tween.eid );
        } );

        remove_elements( characters, MEMFN( flag< IS_DEAD > ) );
        currCharacterIndex %= characters.size();

        deleteEntities( MEMFN( flag< IS_DEAD > ) );
    }

public:

    void update( uint ticks ) override
    {
        Scene::update( ticks );

        inputSystem( ticks );
        behaviorSystem( ticks );

        // Update animations.
        for ( Animation& anim : components< Animation >() )
            anim.update( ticks );

        // Perform tweens.
        for ( auto& tween : posTweens )
            tween( ticks, get< Position >( tween.eid ) );

        deathSystem( ticks );
    }

    void draw() override
    {
        using namespace glm;
        Scene::draw();

        beginFrame();
        useColor( { 1, 1, 1, 1 } );

        // Draw rooms.
#if _MSC_VER >= 1911
        for ( auto[ room, rpos ] : dungeon.enumerate() )
        {
            for ( auto[ tile, tpos ] : room.enumerate() )
            {
                useTexture( tile.getTexture() );
                useSprite( tile.getSprite() );

                vec2 pos = flip_y( rpos + (vec2) tpos ) * TILE_SIZE;
                fillRect( pos, vec2( TILE_SIZE ) );
            }
        }
#else
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
#endif

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

#if _MSC_VER >= 1911
        for ( auto[ eid, pos, tex ] : entities< Position, Texture >() )
        {
            useTexture( (TextureId) tex.textureUnit );
            useSprite( tex.spriteView );
            useColor( tex.color );

            fillRect( pos, vec2( TILE_SIZE ) );
        }

        // Draw healthbars
        for ( auto[ eid, pos, hp, stats ] : entities< Position, HitPoints, Stats >() )
        {
            if ( eid == playerId )
                continue;

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
        }
#else
        invokeSystem( [&]( Eid, Position pos, Texture tex )
        {
            useTexture( (TextureId) tex.textureUnit );
            useSprite( tex.spriteView );
            useColor( tex.color );

            fillRect( pos, vec2( TILE_SIZE ) );
        } );

        invokeSystem( [&]( Eid eid, Position pos, HitPoints hp, Stats stats )
        {
            if ( eid == playerId )
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
#endif

        // Draw Player
        if ( exists( playerId ) )
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

inline Action DungeonScene::AI::operator ()( WanderBehavior& bhvr )
{
    return [&, ntt = ds.entity< Position, Action >( eid )]()
        -> ActionResult
    {
        if ( !ntt ) return true;
        uint ticks = SDL_GetTicks();

        if ( chance( 0.10 ) )
        {
            bhvr.heading += rand_int( 3, 5 );
            bhvr.heading %= 4;
        }

        while ( !bhvr.delay.started() )
        {
            if ( chance( 0.15 ) )
                return true;

            static const vec2 VECTORS[] {
                { 0, 1 }, { 1, 0 }, { 0, -1 }, { -1, 0 },
            };

            vec2 delta = VECTORS[ bhvr.heading ];
            vec2 pos = std::get< 0 >( *ntt ) / TILE_SIZE + delta;

            LevelTile* pTile = ds.dungeon.findTile( pos.x, -pos.y );
            if ( pTile && pTile->tileType == Tile::FLOOR )
            {
                ActionResult result = ds.moveEntity( eid, pTile );

                if ( result.succeeded )
                    bhvr.delay.set( ENEMY_ACTION_DELAY, ticks );

                return result;
            }
            else
            {
                (bhvr.heading += rand_int( 1, 3 )) %= 4;
            }
        }

        if ( bhvr.delay.check( ticks ) )
        {
            bhvr.delay.restart();
            return true;
        }
        return false;
    };
}

inline Action DungeonScene::AI::operator ()( PathBehavior& bhvr )
{
    return [&, eid = eid]() -> ActionResult
    {
        if ( !ds.hasAttached< Position >( eid ) )
            return true;

        uint ticks = SDL_GetTicks();

        while ( !bhvr.delay.started() && ds.exists( bhvr.eid ) )
        {
            Eid tgt = bhvr.eid;
            if ( ds.hasAttached< Position >( tgt ) )
            {
                vec2 pos = ds.get< Position >( eid ) / TILE_SIZE;
                vec2 tgtpos = ds.get< Position >( tgt ) / TILE_SIZE;

                bhvr.path = find_path(
                    ds.dungeon.getTile( pos.x, -pos.y ),
                    ds.dungeon.getTile( tgtpos.x, -tgtpos.y ),
                    ds.dungeon.distanceEstimateFunc(),
                    ds.dungeon.tileCostFunc(),
                    ds.dungeon.neighborsFunc()
                );
                if ( !bhvr.path.empty() )
                    bhvr.path.pop_back();
            }

            if ( bhvr.path.empty() )
                return true;

            auto result = ds.moveEntity( eid, &bhvr.path.back().get() );

            if ( !result.succeeded )
                return result;

            bhvr.delay.set( ENEMY_ACTION_DELAY, ticks );
        }

        if ( bhvr.delay.check( ticks ) )
        {
            bhvr.delay.restart();
            return true;
        }
        return false;
    };
}
