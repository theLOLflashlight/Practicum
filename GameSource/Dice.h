#pragma once

#include "random.h"

#include <iostream>

struct Dice
{
    int numDice = 0;
    int numSides = 0;

    Dice() = default;

    constexpr Dice( int numDice, int numSides )
        : numDice { numDice }
        , numSides { numSides }
    {
    }
};

inline std::ostream& operator <<( std::ostream& out, const Dice& dice )
{
    return out << dice.numDice << "d" << dice.numSides;
}

struct Rollable : Dice
{
    int add;

    constexpr Rollable( const Dice& dice, int add = 0 )
        : Dice( dice )
        , add { add }
    {
    }
};

inline std::ostream& operator <<( std::ostream& out, const Rollable& dice )
{
    return out << (Dice) dice << " + " << dice.add;
}

constexpr Rollable operator +( Rollable dice, int add )
{
    return { dice, dice.add + add };
}

constexpr Rollable operator +( int add, Rollable dice )
{
    return { dice, dice.add + add };
}

inline int roll( const Rollable& dice )
{
    int total = dice.add;
    for ( int i = 0; i < dice.numDice; ++i )
        total += rand_int( 1, dice.numSides );
    return total;
}

#define MAKE_DICE_LITERAL( N ) \
constexpr Dice operator "" d##N( unsigned long long num ) \
{ \
    return { num, N }; \
}

MAKE_DICE_LITERAL( 2 )
MAKE_DICE_LITERAL( 4 )
MAKE_DICE_LITERAL( 6 )
MAKE_DICE_LITERAL( 8 )
MAKE_DICE_LITERAL( 10 )
MAKE_DICE_LITERAL( 12 )
MAKE_DICE_LITERAL( 20 )
MAKE_DICE_LITERAL( 24 )
MAKE_DICE_LITERAL( 30 )
MAKE_DICE_LITERAL( 48 )
MAKE_DICE_LITERAL( 60 )
MAKE_DICE_LITERAL( 120 )

#undef MAKE_DICE_LITERAL
