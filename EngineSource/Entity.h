#pragma once

#include <bitset>

struct Entity
{
    using Bitset = std::bitset< 32 >;

    const int index;
    Bitset    bitset;
    bool isSelected : 1;
    bool isSelectable : 1;

    explicit Entity( int index = 0 )
        : index( index )
        , isSelected( false )
        , isSelectable( true )
    {
    }

    Entity( Entity&& ntt )
        : index( ntt.index )
        , bitset( ntt.bitset )
        , isSelected( ntt.isSelected )
        , isSelectable( ntt.isSelectable )
    {
    }

    Entity& operator =( const Entity& ntt )
    {
        // This is safe because operator= is non-const.
        const_cast< int& >( index ) = ntt.index;
        bitset = ntt.bitset;
        isSelected = ntt.isSelected;
        isSelectable = ntt.isSelectable;
        return *this;
    }

protected:

    template< size_t MaxEntities, typename... Components >
    friend class ComponentManager;

    Entity( const Entity& ntt )
        : index( ntt.index )
        , bitset( ntt.bitset )
        , isSelected( ntt.isSelected )
        , isSelectable( ntt.isSelectable )
    {
    }
};

inline bool operator ==( const Entity& a, const Entity& b )
{
    return a.index == b.index
        && a.bitset == b.bitset
        && a.isSelected == b.isSelected
        && a.isSelectable == b.isSelectable;
}

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

struct PlayerState
{
    int facingDir;
    bool isMoving;
    Delay delay;

    explicit PlayerState( int dir = 0, bool isMoving = false, int delay = 500 )
        : facingDir( dir )
        , isMoving( isMoving )
        , delay( delay )
    {
    }
};
