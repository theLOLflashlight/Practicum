#pragma once

#include "Util.h"

// Specialize this template to support interpolation of different types.
template< typename T >
struct Interpolator;

template<>
struct Interpolator< vec2 > : interp_linear_t {};

template< typename T, typename Interpolator = Interpolator< T > >
struct ValueTweener
{
    uint start;  // start time of tween
    uint finish; // end time of tween
    T    target; // end value of object

    ValueTweener() = default;

    ValueTweener( uint start, uint duration, T target )
        : start( start )
        , finish( start + duration )
        , target( move( target ) )
    {
    }

    void operator ()( uint now, T& obj )
    {
        float frac = scale_point_clamp< float >( now, start, finish, 0, 1 );
        obj = Interpolator{}( obj, target, frac );
        start = now;
    }

    bool expired( uint now ) const
    {
        return now > finish;
    }
};

template< typename T >
struct TweenEntry : ValueTweener< T >
{
    int eid;

    TweenEntry() = default;

    TweenEntry( int eid, T target, uint start, uint duration )
        : ValueTweener< T >( start, duration, move( target ) )
        , eid( eid )
    {
    }
};
