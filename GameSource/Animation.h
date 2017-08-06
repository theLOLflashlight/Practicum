#pragma once

#include "Util.h"

#include <functional>

struct Animation
{
    using UpdateFn = function< void( float ) >;

    uint     start;
    uint     duration;
    UpdateFn updateFn;
    bool     loops;

    Animation() = default;

    Animation( uint     start,
               uint     duration,
               UpdateFn updateFn = nullptr,
               bool     loops = true )
        : start { start }
        , duration { duration }
        , updateFn { move( updateFn ) }
        , loops { loops }
    {
    }

    bool update( uint ticks )
    {
        if ( updateFn != nullptr )
        {
            updateFn( float( ticks - start ) / duration );

            if ( ticks - start > duration )
            {
                if ( loops )
                    start = ticks;
                else
                    updateFn = nullptr;
                return true;
            }
        }
        return false;
    }

    template< typename Func >
    bool update( uint ticks, Func&& callback )
    {
        bool result = update( ticks );
        if ( result == true )
            std::invoke( callback );
        return result;
    }

};
