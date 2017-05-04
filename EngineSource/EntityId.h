// Andrew Meckling
#pragma once
#include <cstdint>
#include <type_traits>
#include <iostream>

// 64bit (8byte) data structure used to uniquely identify 
// entities in the entity-component system.
union EntityId
{
    std::uint64_t   _bitPattern;
    char   _bytes[ 8 ];
    struct {
        std::uint16_t index;
        char          name[ 6 ];
    };

    explicit EntityId( std::uint64_t entityId = 0 );

    EntityId( const char (&tag)[ 2 ], std::uint16_t num = 0 );
    EntityId( const char (&tag)[ 3 ], std::uint16_t num = 0 );
    EntityId( const char (&tag)[ 4 ], std::uint16_t num = 0 );
    EntityId( const char (&tag)[ 5 ], std::uint16_t num = 0 );
    EntityId( const char (&tag)[ 6 ], std::uint16_t num = 0 );
    EntityId( const char (&tag)[ 7 ], std::uint16_t num = 0 );

    EntityId( const char (&tag)[ 8 ] );
    EntityId( const char (&tag)[ 9 ] );

    template< unsigned N, typename = std::enable_if_t< (N <= 7) > >
    bool hasTag( const char (&tag)[ N ] ) const
    {
        static_assert( N > 1, "tag must contain at least 1 character" );
        static_assert( N <= 7, "tag can contain at most 6 characters" );
        return _bitPattern >= EntityId( tag, 0 )._bitPattern
            && _bitPattern <= EntityId( tag, 65535 )._bitPattern;
    }
};

inline std::ostream& operator <<( std::ostream& os, const EntityId& eid )
{
    char name[ 7 ] = { 0 };
    memcpy( name, eid.name, 6 );
    return os << eid._bitPattern << "(" << name << ":" << eid.index << ")";
}

inline bool operator ==( const EntityId& lhs, const EntityId& rhs )
{
    return lhs._bitPattern == rhs._bitPattern;
}

inline bool operator !=( const EntityId& lhs, const EntityId& rhs )
{
    return lhs._bitPattern != rhs._bitPattern;
}

inline bool operator <( const EntityId& lhs, const EntityId& rhs )
{
    return lhs._bitPattern < rhs._bitPattern;
}

inline bool operator >( const EntityId& lhs, const EntityId& rhs )
{
    return lhs._bitPattern > rhs._bitPattern;
}

inline bool operator <=( const EntityId& lhs, const EntityId& rhs )
{
    return lhs._bitPattern <= rhs._bitPattern;
}

inline bool operator >=( const EntityId& lhs, const EntityId& rhs )
{
    return lhs._bitPattern >= rhs._bitPattern;
}

namespace std
{
    template<>
    struct hash< EntityId >
    {
        using argument_type = EntityId;
        using result_type   = std::size_t;

        result_type operator ()( const argument_type& eid ) const
        {
            return std::hash< std::uint64_t >{}( eid._bitPattern );
        }
    };
}
