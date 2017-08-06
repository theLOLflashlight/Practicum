#pragma once

#include "Util.h"
#include "Player.h"
#include "Dungeon.h"
#include "Delay.h"

#include <variant>
#include <vector>

struct AbstractBehavior
{
};

struct IdleBehavior
{
};

struct WanderBehavior
{
    int   heading;
    Delay delay;

    explicit WanderBehavior( int heading )
        : heading { heading }
    {
    }
};

struct PathBehavior
{
    Eid eid;
    RefVector< LevelTile > path;
    Delay delay;

    explicit PathBehavior( Eid eid )
        : eid { eid }
    {
    }
};

template< typename... Behaviors >
using VariadicBehavior = std::variant< AbstractBehavior, Behaviors... >;
