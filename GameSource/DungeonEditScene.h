#pragma once

#include "DungeonScene.h"

#include <fstream>

class DungeonEditScene
    : public DungeonScene
{
private:

public:

    explicit DungeonEditScene( SDL_Window* pWindow )
        : DungeonScene( pWindow )
    {
    }

    void update( uint ticks ) override
    {
        DungeonScene::update( ticks );

        vec2 mPos = getMousePos() / (RENDER_SCALE * TILE_SIZE);
        LevelTile* pTile = dungeon.findTile( mPos.x, mPos.y );

        if ( pTile != nullptr )
        {
            if ( wasButtonPressed( LEFT_BUTTON ) )
            {
                *pTile = FLOOR_TILE;
                updateConnections();
                initExtras();
            }
            else if ( wasButtonPressed( RIGHT_BUTTON ) )
            {
                *pTile = WALL_TILE;
                updateConnections();
                initExtras();
            }
            else if ( wasButtonPressed( MIDDLE_BUTTON ) )
            {
                *pTile = PIT_TILE;
                updateConnections();
                initExtras();
            }
        }

        // Ctrl + S (save) behaviour
        if ( isCtrlDown() && wasKeyPressed( SDLK_s ) )
            saveDungeon();

        // Ctrl + O (open) behaviour
        if ( isCtrlDown() && wasKeyPressed( SDLK_o ) )
            loadDungeon();
    }

    void draw() override
    {
        using namespace glm;
        DungeonScene::draw();

        beginFrame();

        {
            ivec2 mouseTile = getMousePos() / RENDER_SCALE;
            // Round to tile size.
            mouseTile /= TILE_SIZE;
            mouseTile *= TILE_SIZE;

            useTexture( DEFAULT );
            useColor( { 1, 1, 1, 0.3 } );
            useSprite( { 0, 0, 1, 1 } );
            fillRect( flip_y( mouseTile ), vec2( TILE_SIZE ) );
        }

        endFrame();
    }

protected:

    void saveDungeon()
    {
        using std::endl;

        std::ofstream file( "dungeon.txt", std::ios_base::trunc );

        file << dungeon.roomCount() << endl;

        dungeon.eachRoom( [&]( Room& room, vec2 pos )
        {
            file << pos.x << ' ' << pos.y << endl;
            file << room.width() << ' ' << room.height() << endl;

            room.eachTile( [&]( LevelTile& tile, ivec2 )
            {
                file << (int) tile.tileType << ' '
                    << tile.offset.x << ' '
                    << tile.offset.y << ' ';
            } );

            file << endl;
        } );
    }

    void loadDungeon()
    {
        std::ifstream file( "dungeon.txt" );

        int numRooms;
        file >> numRooms;

        Dungeon newDungeon;

        for ( int i = 0; i < numRooms; ++i )
        {
            vec2 pos;
            file >> pos.x >> pos.y;

            ivec2 size;
            file >> size.x >> size.y;
            Room room( size.x, size.y );

            room.eachTile( [&]( LevelTile& tile, ivec2 )
            {
                Tile type;
                ivec2 off;
                file >> (int&) type >> off.x >> off.y;

                tile = LevelTile( type, off );
            } );

            newDungeon.addRoom( move( room ), pos );
        }

        dungeon = move( newDungeon );
        updateConnections();
        initExtras();
    }

};
