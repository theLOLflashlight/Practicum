#pragma once

#include "Util.h"
#include "Dungeon.h"
#include "Texture.h"

#include <memory>

using HitPoints = int;

enum Obstruction : unsigned char
{
    NONE,
    GROUND = 0b01,
    AIR = 0b10,
    TALL = GROUND & AIR,
    AQUATIC = 0b100,
    AMPHIBIOUS = AQUATIC & GROUND,
    CAVE = AQUATIC & AIR,
    ALL = 0b111,
};

struct Stats
{
    int maxHealth;
    int attack;
    int magick;
    int speed;
    Obstruction blocks;

    operator Obstruction() const
    {
        return blocks;
    }
};

constexpr bool overlaps( Obstruction a, Obstruction b )
{
    return (a & b) != 0;
}

constexpr Stats WARRIOR_STATS { 7, 4, 2, 3, Obstruction::GROUND };
constexpr Stats ROGUE_STATS { 5, 3, 3, 5, Obstruction::GROUND };
constexpr Stats MAGE_STATS { 5, 3, 5, 3, Obstruction::GROUND };
constexpr Stats PALADIN_STATS { 6, 4, 3, 3, Obstruction::GROUND };


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

struct ActionResult;

using Action = function< ActionResult() >;

struct ActionResult
{
    bool   succeeded;
    Action backupAction;

    ActionResult( bool success, Action backup = {} )
        : succeeded { success }
        , backupAction { move( backup ) }
    {
    }

    ActionResult( function< ActionResult() > func )
        : succeeded { false }
        , backupAction { move( func ) }
    {
    }
};

ActionResult perform( Action& action )
{
    return action != nullptr ? action() : ActionResult( true );
}
