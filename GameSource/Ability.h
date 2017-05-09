
#include "Util.h"

#include <tuple>

enum Direction
{
    UP, RIGHT, DOWN, LEFT
};


class Ability
{
public:

    virtual void doAbility( Direction dir )
    {
    }
};

struct WallOfFireSpell : Ability
{
    void doAbility( Direction dir ) override
    {
    }
};

struct ChainLightningSpell : Ability
{
    void doAbility( Direction dir ) override
    {
    }
};

struct ConeOfColdSpell : Ability
{
    void doAbility( Direction dir ) override
    {
    }
};

enum AttackId
{
    WALL_OF_FIRE,
    CHAIN_LIGHTNING,
    CONE_OF_COLD,
};

class Attack
{
private:

    AttackId _attackId;

    union // Attack
    {
        Ability             _attack;
        WallOfFireSpell     _wallOfFire;
        ChainLightningSpell _chainLightning;
        ConeOfColdSpell     _coneOfCold;
    };

public:

    Attack( WallOfFireSpell wallOfFire )
        : _attackId { WALL_OF_FIRE }
        , _wallOfFire { move( wallOfFire ) }
    {
    }

    Attack( ChainLightningSpell chainLightning )
        : _attackId { CHAIN_LIGHTNING }
        , _chainLightning { move( chainLightning ) }
    {
    }

    Attack( ConeOfColdSpell coneOfCold )
        : _attackId { CONE_OF_COLD }
        , _coneOfCold { move( coneOfCold ) }
    {
    }

    AttackId id() const
    {
        return _attackId;
    }

    template< typename Func >
    auto visit( Func&& func )
    {
        using std::invoke;

        switch ( _attackId )
        {
        case WALL_OF_FIRE: return invoke( func, _wallOfFire );
        case CHAIN_LIGHTNING: return invoke( func, _chainLightning );
        case CONE_OF_COLD: return invoke( func, _coneOfCold );
        default:
            printf( "Unknown AttackId\n" );
            return invoke( func, _attack );
        }
    }
    
    Ability* operator ->()
    {
        switch ( _attackId )
        {
        case WALL_OF_FIRE: return &_wallOfFire;
        case CHAIN_LIGHTNING: return &_chainLightning;
        case CONE_OF_COLD: return &_coneOfCold;
        default:
            printf( "Unknown AttackId\n" );
            return &_attack;
        }
    }


};

