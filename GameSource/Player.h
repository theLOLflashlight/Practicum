#pragma once

#include "Util.h"
#include "Dungeon.h"

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
};

struct ActionResult;


struct Action
{
    std::function< ActionResult() > fnAction;

    ActionResult perform();

	Action( std::function< ActionResult() > func = nullptr )
		: fnAction{ move ( func ) }
	{
	}

	operator bool() const
	{
		return fnAction == nullptr;
	}
};

struct ActionResult
{
    bool   succeeded;
    Action backupAction;

	explicit ActionResult( bool success, Action backup = {} )
		: succeeded { success }
		, backupAction { move( backup ) }
	{
	}
};

ActionResult Action::perform()
{
	return fnAction ? fnAction() : ActionResult( true );
}

class PlayerGlue
    //: public Stats
{
public:

    //constexpr PlayerGlue( Stats stats = {} )
    //    : Stats( move( stats ) )
    //{
    //}

    virtual ~PlayerGlue() {}

    virtual Position& position() = 0;

    virtual HitPoints& health() = 0;

    virtual Texture& texture() = 0;

    virtual Stats& stats() = 0;

	virtual Animation& animation() = 0;

	virtual Action& action() = 0;

    virtual Entity& entity() = 0;
};

class Player
{
    std::unique_ptr< PlayerGlue > glue;

public:

    template< typename Glue >
    Player( const Glue& glue )
        : glue { new Glue( glue ) }
    {
    }

    #define PLAYER_FUNC( FUNCTION )    \
    decltype(auto) FUNCTION()          \
    {                                  \
        return glue->FUNCTION();       \
    }

    PLAYER_FUNC( position );
    PLAYER_FUNC( health );
    PLAYER_FUNC( texture );
    PLAYER_FUNC( stats );
	PLAYER_FUNC( animation );
	PLAYER_FUNC( action );
    PLAYER_FUNC( entity );
    
    #undef PLAYER_FUNC
};