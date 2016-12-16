#pragma once

template< typename Itr, typename T >
Itr binary_search( Itr first, Itr last, const T& target )
{
    using namespace std;
    auto lower = first;
    auto upper = last;

    while ( lower <= upper )
    {
        auto midle = lower + (upper - lower) / 2;
        
        if ( *midle < target )
            lower = midle + 1;
        else if ( *midle > target )
            upper = midle - 1;
        else
            return midle;
    }

    return last;
}

template< typename Cont, typename T >
auto binary_search( const Cont& cont, const T& target )
{
    return binary_search( std::begin( cont ), std::end( cont ), target );
}
