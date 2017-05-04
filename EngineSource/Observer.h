// Andrew Meckling
#pragma once

#include "Dictionary.h"
#include "Util.h"

#include <functional>
#include <vector>
#include <typeinfo>
#include <map>

// Subject used in the observer pattern.
template< typename Event, typename Func >
class Subject;

// Subject used in the observer pattern.
template< typename Event, typename... Args >
class Subject< Event, void( Args... ) >
{
public:

    // Type of the observer function.
    using FuncType = std::function< void( Args... ) >;

    struct Object
    {
        void*    ptr = nullptr;
        FuncType func = nullptr;

        Object() = default;

        Object( FuncType func )
            : func( std::move( func ) )
        {
        }

        Object( void* ptr, FuncType func )
            : ptr( ptr )
            , func( std::move( func ) )
        {
        }
    };

private:

    

    // Maps events to observers.
    Dictionary< Event, std::vector< FuncType > > _observers;

public:

    // Registers an observer to receive callbacks when a particular
    // event is triggered.
    template< typename Observer >
    void registerObserver( const Event& event, Observer&& observer )
    {
        _observers[ event ].push_back( forward< Observer >( observer ) );
    }

    // Unregisters all observers of the same underlying type from a 
    // particular event.
    template< typename Observer >
    void unregisterObserver( const Event& event, Observer&& observer )
    {
        using namespace std;
        auto& obsvs = _observers[ event ];

        auto last = remove_if(
            begin( obsvs ), end( obsvs ),
            [&]( const FuncType& fn )
            {
                return typeid( Observer ) == fn.target_type();
            } );

        obsvs.erase( last, end( obsvs ) );
    }

    // Notifies the observers of a particular event, passing on 
    // the additional arguments to each observer.
    void notify( const Event& event, Args&&... args )
    {
        for ( auto& obs : _observers[ event ] )
            std::invoke( obs, forward< Args >( args )... );
    }

};
