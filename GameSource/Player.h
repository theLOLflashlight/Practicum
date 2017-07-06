#pragma once

#include "Util.h"
#include "Dungeon.h"

#include <memory>

using HitPoints = int;

struct Stats
{
    int maxHealth;
    int attack;
    int magick;
    int speed;
};

class PlayerGlue
{
public:

    virtual ~PlayerGlue() {}

    virtual Position& position() = 0;

    virtual HitPoints& health() = 0;

    virtual Texture& texture() = 0;

    virtual Entity& entity() = 0;
};

class Player
{
    std::unique_ptr< PlayerGlue > glue;

public:

    Stats stats;

    template< typename Glue >
    Player( Stats stats, const Glue& glue )
        : glue { new Glue( glue ) }
        , stats { stats }
    {
        health() = stats.maxHealth;
    }

    #define PLAYER_FUNC( FUNCTION )    \
    decltype(auto) FUNCTION()          \
    {                                  \
        return glue->FUNCTION();       \
    }

    PLAYER_FUNC( position );
    PLAYER_FUNC( health );
    PLAYER_FUNC( texture );
    
    #undef PLAYER_FUNC
};
