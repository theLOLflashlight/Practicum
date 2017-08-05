#pragma once

#include <optional>

template< typename T >
struct optional_or_void
{
    using type = std::optional< T >;
};

template<>
struct optional_or_void< void >
{
    struct type
    {
        template< typename T > auto& operator =( T&& ) { return *this; }
        template< typename T > operator T() const { return {}; }
    };
};

template< typename T >
using optional_or_void_t = typename optional_or_void< T >::type;

class Delay
{
public:

    unsigned delay;
    unsigned start = -1;

    Delay() = default;

    explicit Delay( unsigned delay, unsigned start = -1 )
        : delay( delay )
        , start( start )
    {
    }

    bool started() const
    {
        return start != -1;
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
    auto triggerIf( unsigned ticks, bool cond, Func&& func )
        -> optional_or_void_t< decltype( func() ) >
    {
        optional_or_void_t< decltype( func() ) > result;
        if ( !cond )
        {
            restart( ticks );
        }
        else if ( check( ticks ) )
        {
            result = func();
            restart( ticks );
        }
        return result;
    }
};
