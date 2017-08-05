#pragma once

#include "Util.h"
#include "Player.h"
#include "Dungeon.h"
#include "Delay.h"

#include <variant>

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
};

template< typename... Behaviors >
using VariadicBehavior = std::variant< AbstractBehavior, Behaviors... >;
