#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <string>
#include <bitset>
#include <array>
#include <initializer_list>
#include <cassert>

template<
    typename KeyType,
    typename HashFn = std::hash< KeyType > >
class HashSet
{
public:
    using Key = KeyType;

    static constexpr double LOAD_FACTOR = 0.75;
    static constexpr double GROWTH_FACTOR = 2.0;

    struct Bucket
    {
        std::vector< Key > entries;

        bool empty() const
        {
            return entries.empty();
        }

        size_t count() const
        {
            return entries.size();
        }

        Key& last()
        {
            return entries.back();
        }

        Key* find( const Key& key )
        {
            for ( Key& key : entries )
                if ( key == key )
                    return &key;
            return nullptr;
        }

        const Key* find( const Key& key ) const
        {
            return const_cast< Bucket& >( *this ).find( key );
        }

        Key* insert( Key key )
        {
            if ( find( key ) != nullptr )
                return nullptr;
            entries.emplace_back( std::move( key ) );
            return &entries.back();
        }

        Key remove( const Key& key )
        {
            if ( Key* pKey = find( key ) )
            {
                Key entry = std::move( *pKey );
                auto begin = entries.begin();
                entries.erase( begin + std::distance( &*begin, pKey ) );
                return entry;
            }
            return {};
        }
    };

    using Table = std::vector< Bucket >;

private:

    Table table;

public:

    HashSet( size_t size = 50 )
        : table( size )
    {
    }

    HashSet( std::initializer_list< Key > il, double factor = GROWTH_FACTOR )
        : table( size_t( std::size( il ) * factor ) )
    {
        for ( const Key& key : il )
            insert( std::move( key ) );
    }

    size_t hash( const Key& key ) const
    {
        return HashFn{}( key ) % size();
    }

    void clear()
    {
        table.clear();
    }

    size_t size() const
    {
        return table.size();
    }

    size_t count() const
    {
        size_t num = 0;
        for ( const Bucket& cell : table )
            num += cell.count();
        return num;
    }

    Key* find( const Key& key )
    {
        return table[ hash( key ) ].find( key );
    }

    Key remove( const Key& key )
    {
        return table[ hash( key ) ].remove( key );
    }

    const Key* insert( Key key )
    {
        table[ hash( key ) ].insert( std::move( value ) );

        if ( count() > LOAD_FACTOR * size() )
            rehash( size() * GROWTH_FACTOR );
        
        return table[ hash( key ) ].find( key );
    }

    void rehash( size_t size )
    {
        HashSet newSet( size );

        for ( Bucket& cell : table )
            for ( Key& key : cell.entries )
                newSet.insert( std::move( key.value ) );

        table.swap( newSet.table );
    }

    template< typename Func >
    void forEach( Func&& func )
    {
        for ( Bucket& cell : table )
            for ( Key& key : cell.entries )
                func( key );
    }

};