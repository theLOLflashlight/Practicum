#pragma once

class Delay
{
public:

    unsigned delay;
    unsigned start;

    explicit Delay( unsigned delay, unsigned start = -1 )
        : delay( delay )
        , start( start )
    {
    }

    void restart( unsigned start = -1 )
    {
        this->start = start;
    }

    void set( unsigned delay, unsigned start )
    {
        this->delay = delay;
        this->start = start;
    }

    bool check( unsigned ticks ) const
    {
        return ticks - start > delay;
    }

    template< typename Func >
    void triggerIf( unsigned ticks, bool cond, Func&& func )
    {
        if ( !cond )
        {
            restart( ticks );
        }
        else if ( check( ticks ) )
        {
            func();
            restart( ticks );
        }
    }
};
