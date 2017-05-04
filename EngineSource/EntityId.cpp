// Andrew Meckling
#include "EntityId.h"
#include <cstring>

using namespace std;

EntityId::EntityId( uint64_t entityId )
    : _bitPattern( entityId )
{
}


EntityId::EntityId( const char( &entityTag )[ 2 ], uint16_t entityNum )
    : _bitPattern( static_cast< uint64_t >( entityNum ) )
{
    memcpy( name, entityTag, 1 );
}

EntityId::EntityId( const char( &entityTag )[ 3 ], uint16_t entityNum )
    : _bitPattern( static_cast< uint64_t >( entityNum ) )
{
    memcpy( name, entityTag, 2 );
}

EntityId::EntityId( const char( &entityTag )[ 4 ], uint16_t entityNum )
    : _bitPattern( static_cast< uint64_t >( entityNum ) )
{
    memcpy( name, entityTag, 3 );
}

EntityId::EntityId( const char( &entityTag )[ 5 ], uint16_t entityNum )
    : _bitPattern( static_cast< uint64_t >( entityNum ) )
{
    memcpy( name, entityTag, 4 );
}

EntityId::EntityId( const char( &entityTag )[ 6 ], uint16_t entityNum )
    : _bitPattern( static_cast< uint64_t >( entityNum ) )
{
    memcpy( name, entityTag, 5 );
}

EntityId::EntityId( const char( &entityTag )[ 7 ], uint16_t entityNum )
    : _bitPattern( static_cast< uint64_t >( entityNum ) )
{
    memcpy( name, entityTag, 6 );
}


EntityId::EntityId( const char( &entityTag )[ 8 ] )
    : _bitPattern( static_cast< uint64_t >( 0 ) )
{
    memcpy( &_bytes[ 1 ], entityTag, 7 );
}

EntityId::EntityId( const char( &entityTag )[ 9 ] )
    : _bitPattern( static_cast< uint64_t >( 0 ) )
{
    memcpy( &_bytes[ 0 ], entityTag, 8 );
}