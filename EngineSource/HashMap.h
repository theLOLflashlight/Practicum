#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <string>
#include <bitset>
#include <array>
#include <initializer_list>
#include <cassert>

template< uint64_t X = 5381, uint64_t Y = 33 >
constexpr uint64_t myHash( const char* s, int off = 0 )
{                        
    return !s[ off ] ? X : (myHash< X, Y >( s, off + 1 ) * Y) ^ s[ off ];                           
}

inline uint64_t hash1( const std::string& key )
{
    return myHash( key.c_str() );
}

// FNV-1a hash routine for string objects.
inline uint64_t hash2( const std::string& key )
{
    uint64_t hashVal = 2166136261;

    for ( char ch : key )
        hashVal = (hashVal ^ ch) * 16777619;

    return hashVal;
}

template<
    typename KeyType,
    typename ValueType,
    typename HashFn = std::hash< KeyType > >
class HashMap
{
public:
    using Key = KeyType;
    using Value = ValueType;

    static constexpr double LOAD_FACTOR = 0.75;
    static constexpr double GROWTH_FACTOR = 2.0;

    struct Entry
    {
        Key   key;
        Value value;
    };

    struct Bucket
    {
        std::vector< Entry > entries;

        bool empty() const
        {
            return entries.empty();
        }

        size_t count() const
        {
            return entries.size();
        }

        Entry& last()
        {
            return entries.back();
        }

        Entry* find( const Key& key )
        {
            for ( Entry& entry : entries )
                if ( entry.key == key )
                    return &entry;
            return nullptr;
        }

        const Entry* find( const Key& key ) const
        {
            return const_cast< Bucket& >( *this ).find( key );
        }

        Entry* insert( Entry newEntry )
        {
            if ( find( newEntry.key ) != nullptr )
                return nullptr;
            entries.emplace_back( std::move( newEntry ) );
            return &entries.back();
        }

        Entry remove( const Key& key )
        {
            if ( Entry* pEntry = find( key ) )
            {
                Entry entry = std::move( *pEntry );
                auto begin = entries.begin();
                entries.erase( begin + std::distance( &*begin, pEntry ) );
                return entry;
            }
            return {};
        }
    };

    using Table = std::vector< Bucket >;

private:

    Table table;

public:

    HashMap( size_t size = 50 )
        : table( size )
    {
    }

    HashMap( std::initializer_list< Entry > il, double factor = GROWTH_FACTOR )
        : table( size_t( std::size( il ) * factor ) )
    {
        for ( const Entry& entry : il )
            insert( entry.key, std::move( entry.value ) );
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

    Value* find( const Key& key )
    {
        return table[ hash( key ) ].find( key );
    }

    Value remove( const Key& key )
    {
        return table[ hash( key ) ].remove( key ).value;
    }

    const Entry* insert( const Key& key, Value value )
    {
        table[ hash( key ) ].insert( { key, std::move( value ) } );

        if ( count() > LOAD_FACTOR * size() )
            rehash( size() * GROWTH_FACTOR );
        
        return table[ hash( key ) ].find( key );
    }

    void rehash( size_t size )
    {
        HashMap newMap( size );

        for ( Bucket& cell : table )
            for ( Entry& entry : cell.entries )
                newMap.insert( entry.key, std::move( entry.value ) );

        table.swap( newMap.table );
    }

    Value& operator []( const Key& key )
    {
        Bucket& cell = table[ hash( key ) ];
        Entry* pEntry = cell.find( key );

        if ( pEntry == nullptr )
        {
            cell.insert( { key, {} } );
            
            if ( count() > LOAD_FACTOR * size() )
                rehash( size() * GROWTH_FACTOR );

            return table[ hash( key ) ].last().value;
        }
        return pEntry->value;
    }
    
    const Value& operator []( const Key& key ) const
    {
        Value* pValue = find( key );
        assert( pValue != nullptr );
        return *pValue;
    }

    template< typename Func >
    void forEach( Func&& func )
    {
        for ( Bucket& cell : table )
            for ( Entry& entry : cell.entries )
                func( entry );
    }

};