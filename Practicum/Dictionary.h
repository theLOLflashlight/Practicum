// Andrew Meckling
#pragma once

#include <algorithm>
#include <initializer_list>

// Convenience definition used by Dictionary.
// Useful for the initializer_list constructor.
template< typename KeyType, typename ValueType >
struct MapEntry
{
    KeyType   key;
    ValueType value;
};

// A dense binary searching dictionary object. (Allocates memory.)
// Supports move, copy, swap, and iteration operations.
// All keys and all values are stored contiguously.
// Datawise, the allocated memory looks like this:
//     struct AllocatedMemory {
//         KeyType keys[ N ];
//         ValueType values[ N ];
//     };
// where N is the capacity of the dictionary.
// The elements stored by this container are sorted on their keys.
// This is to allow key lookup to utilize a binary search. This 
// dramatically improves lookup performance at the cost of insertion
// /deletion performance.
template< typename KeyType_, typename ValueType_ >
class Dictionary
{
public:

    using KeyType   = KeyType_;
    using ValueType = ValueType_;

    static constexpr size_t KEY_SIZE   = sizeof( KeyType );
    static constexpr size_t VALUE_SIZE = sizeof( ValueType );

    template< typename T >
    struct _iterator;

    using Iterator = _iterator< ValueType >;
    using ConstIterator = _iterator< const ValueType >;

    using EntryType           = MapEntry< KeyType, ValueType >;
    using InitializerListType = std::initializer_list< EntryType >;

private:

    void*  _pData;      // Pointer to the allocated memory.
    size_t _capacity;   // Maximum number of entries that can be stored by _pData.
    size_t _count;      // Current number of entries that are stored by _pData.

    friend void std::swap( Dictionary& a, Dictionary& b );

    // Allocates a block of memory for the keys and values.
    static void* _allocate( size_t capacity )
    {
        return new char[ (KEY_SIZE + VALUE_SIZE) * capacity ];
    }

    // Deallocates a block of memory used by the keys and values.
    static void _deallocate( void* pData )
    {
        delete[] pData;
    }

    #define _pKeys _keys()

    // Gets a pointer to the keys.
    KeyType* _keys()
    {
        return reinterpret_cast< KeyType* >( _pData );
    }

    // Gets a const pointer to the keys.
    const KeyType* _keys() const
    {
        return reinterpret_cast< const KeyType* >( _pData );
    }

    #define _pValues _values()

    // Gets a pointer to the values.
    ValueType* _values()
    {
        return reinterpret_cast< ValueType* >( _pKeys + _capacity );
    }

    // Gets a const pointer to the values.
    const ValueType* _values() const
    {
        return reinterpret_cast< const ValueType* >( _pKeys + _capacity );
    }

    // Constructs a key and a value at a specific index in the map.
    void _construct( size_t pos, KeyType key, ValueType value )
    {
        new(_pKeys + pos) KeyType( std::move( key ) );
        new(_pValues + pos) ValueType( std::move( value ) );
    }

    // Destroys a key and a value at a specific index in the map.
    void _destroy( size_t pos )
    {
        _pKeys[ pos ].~KeyType();
        _pValues[ pos ].~ValueType();
    }

public:

    // Allocates memory for the table of keys and values.
    // Default capacity is 12; capacity must be greater than 0.
    explicit Dictionary( size_t capacity = 12 )
        : _pData( _allocate( capacity ) )
        , _capacity( capacity )
        , _count( 0 )
    {
    }

    // Allocates memory for the supplied table, plus an optional amount of padding.
    // The provided initializer_list does not need to be in any order, however,
    // supplying duplicate keys will result in undefined behaviour.
    Dictionary( InitializerListType list, size_t padding = 0 )
        : Dictionary( list.size() + padding )
    {
        _count = list.size();

        // if the supplied table is already sorted we can skip sorting it later
        bool sorted = true;
        const EntryType* prev = nullptr;
        for ( const EntryType& entry : list )
        {
            if ( sorted && prev != nullptr && entry.key < prev->key )
                sorted = false;
            new( _pKeys + (&entry - list.begin()) ) KeyType( entry.key );
            prev = &entry;
        }

        if ( sorted )
        {   // slot the values into place
            for ( const EntryType& entry : list )
                new( _pValues + (&entry - list.begin()) ) ValueType( std::move( entry.value ) );
        }
        else
        {   // sort then search for the correct spot for each value
            std::sort( _pKeys, _pKeys + _count );
            for ( const EntryType& entry : list )
                new( search( entry.key ) ) ValueType( std::move( entry.value ) );
        }
    }

    // Copies the contents of a map into the constructed map.
    Dictionary( const Dictionary& copy )
        : Dictionary( copy._capacity )
    {
        _count = copy._count;
        for ( size_t i = 0; i < copy._count; ++i )
            _construct( i, copy._pKeys[ i ], copy._pValues[ i ] );
    }

    // Moves the contents from a map into the constructed map.
    Dictionary( Dictionary&& move )
        : _pData( move._pData )
        , _capacity( move._capacity )
        , _count( move._count )
    {
        move._pData = nullptr;
        move._capacity = 0;
        move._count = 0;
    }

    // Copies the contents of one map into another map.
    Dictionary& operator =( const Dictionary& copy )
    {
        if ( _capacity != copy._capacity )
        {
            clear();
            _deallocate( _pData );
            _pData = _allocate( copy._capacity );
            _capacity = copy._capacity;

            for ( size_t i = 0; i < copy._count; ++i )
                _construct( i, copy._pKeys[ i ], copy._pValues[ i ] );
        }
        else
        {
            size_t i;
            for ( i = 0; i < copy._count; ++i )
            {
                if ( i < _count )
                    _destroy( i );
                _construct( i, copy._pKeys[ i ], copy._pValues[ i ] );
            }
            while ( i < _count )
                _destroy( i++ );
        }

        _count = copy._count;
        return *this;
    }

    // Moves the contents from one map into another map.
    Dictionary& operator =( Dictionary&& move )
    {
        std::swap( *this, move );
        return *this;
    }

    // Clears and deallocates the map.
    ~Dictionary()
    {
        clear();
        _deallocate( _pData );
    }

    // Returns the number of entries in the map.
    size_t count() const
    {
        return _count;
    }

    // Returns the number of entries that can be held without 
    // automatically reallocating memory.
    size_t capacity() const
    {
        return _capacity;
    }

    // Returns the size of the allocated memory in bytes.
    size_t size() const
    {
        return (KEY_SIZE + VALUE_SIZE) * _capacity;
    }

    // Empties the map without deallocating memory.
    void clear()
    {
        while ( _count > 0 )
            _destroy( --_count );
    }

    // Accesses the value mapped to the given key.
    ValueType& operator []( const KeyType& key )
    {
        Iterator itr = search( key );
        return itr ? *itr : *_insert( itr._index( this ), key, ValueType {} );
    }

    // Accesses the value mapped to the given key.
    const ValueType& operator []( const KeyType& key ) const
    {
        ConstIterator itr = search( key );
        return itr ? *itr : throw "key not found";
    }

    // Performs a binary search over the keys.
    Iterator search( KeyType key )
    {
        const KeyType* lower = _pKeys;
        const KeyType* upper = (_pKeys + _count) - 1;
        const KeyType* mid   = _pKeys + (_count / 2);

        while ( lower <= upper )
        {
            if ( *mid < key ) lower = mid + 1;
        else if ( *mid > key ) upper = mid - 1;
            else return Iterator( this, mid );

            mid = lower + (upper - lower) / 2;
        }

        return Iterator( nullptr, mid );
    }

    // Performs a binary search over the keys.
    ConstIterator search( KeyType key ) const
    {
        return const_cast< Dictionary* >( this )->search( key );
    }

    // Allocates memory, moves entries, then deallocates memory.
    void reallocate( size_t size )
    {
        _reallocateSplit( size, 0, 0 );
    }

    // Adds an entry to the map, unless the key is already contained in the map.
    // Returns a pointer to the newly added value on success, otherwise nullptr.
    ValueType* add( KeyType key, ValueType value )
    {
        Iterator itr = search( key );
        if ( itr )
            return nullptr;

        return _insert( itr._index( this ), key, std::move( value ) );
    }

    // Removes an entry from the map. Returns true if an entry was found, otherwise false.
    bool remove( KeyType key )
    {
        Iterator itr = search( key );
        if ( !itr )
            return false;

        _erase( itr.index() );
        return true;
    }

private:

    // Allocates memory, moves entries, then deallocates memory.
    // The new allocation will have splitSize number of unconstructed entries
    // inserted after splitPos. The user of this function is respoinsible for 
    // constructing valid entries into the 'gap' left in the reallocated memory.
    void _reallocateSplit( size_t size, size_t splitPos, size_t splitSize = 1 )
    {
        // Avoid reallocating if possible.
        if ( size == _capacity )
            return;

        Dictionary tmp( size ); // Holder for old map data.
        tmp._count = std::min( _count, size );

        std::swap( *this, tmp );
        // Pay careful attention when trying to understand the meaning
        // of the code after the swap.

        // Move old map data back into the current map.
        for ( size_t i = 0; i < _count; ++i )
            _construct( i < splitPos ? i : i + splitSize,
                        std::move( tmp._pKeys[ i ] ),
                        std::move( tmp._pValues[ i ] ) );

        // Destroy untouched elements.
        for ( size_t i = _count; i < tmp._count; ++i )
            tmp._destroy( i );

        tmp._count = 0; // tmp contains no valid entries.
        // tmp's dtor cleans up old memory allocation.
    }

    // Inserts a key and a value at a specific index in the map.
    // Reallocates memory if necessary. Returns an iterator to the
    // newly inserted entry.
    Iterator _insert( size_t pos, KeyType key, ValueType value )
    {
        using std::move;

        if ( pos > _count )
            throw "index out of bounds";

        if ( _count == _capacity )
        {
            if ( pos == _count ) // Appending
                reallocate( _capacity * 2 );
            else
                _reallocateSplit( _capacity * 2, pos );
        }
        else // Shift chunk of data right when not appending
        {
            for ( size_t i = _count; i > pos; --i )
            {
                _pKeys[ i ]   = move( _pKeys[ i - 1 ] );
                _pValues[ i ] = move( _pValues[ i - 1 ] );
            }
        }

        _construct( pos, move( key ), move( value ) );
        ++_count;

        return Iterator( this, _pKeys + pos );
    }

    // Removes an entry at a specific index from the map. Returns an 
    // iterator to the next entry.
    Iterator _erase( size_t pos, size_t count = 1 )
    {
        if ( pos >= _count )
            throw "index out of bounds";

        // Shift chunk of data left when erasing from middle
        for ( size_t i = pos; i < _count - count; ++i )
        {
            _pKeys[ i ]   = std::move( _pKeys[ i + count ] );
            _pValues[ i ] = std::move( _pValues[ i + count ] );
        }

        for ( size_t i = _count - count; i < _count; ++i )
            _destroy( i );

        _count -= count;

        return Iterator( pos < _count ? this : nullptr, _pKeys + pos );
    }

public:

    #pragma region Iterators

    template< typename T >
    struct _iterator
    {
        using ValueType = T;

        // Holds references to the key and value associated
        // with the same entry in a map.
        struct EntryProxy
        {
            const KeyType& key;   // Const reference to the key pointed to by the iterator.
            ValueType&     value; // Reference to the value pointed to by the iterator.

            operator ValueType&()
            {
                return value;
            }
        };

        const KeyType* pKey;

        Dictionary* const _pMap;

        _iterator( Dictionary* this_map, const KeyType* pKey )
            : pKey( pKey )
            , _pMap( this_map )
        {
        }

        _iterator( const _iterator< std::remove_const_t< ValueType > >& itr )
            : pKey( itr.pKey )
            , _pMap( itr._pMap )
        {
        }

        size_t index() const
        {
            return _index( _pMap );
        }

        size_t _index( const Dictionary* pMap ) const
        {
            ptrdiff_t diff = pKey - pMap->_pKeys;
            #ifdef _DEBUG
            if ( diff < 0 || size_t( diff ) > pMap->_capacity )
                throw "key does not belong to map";
            #endif
            return diff;
        }

        #pragma region Operators

        // Also handles conversion to bool.
        operator ValueType*() const
        {
            return _pMap ? _pMap->_pValues + index() : nullptr;
        }

        EntryProxy operator *() const
        {
            return { *pKey, _pMap->_pValues[ index() ] };
        }

        ValueType* operator ->() const
        {
            return _pMap->_pValues + index();
        }

        auto operator ++()
        {
            ++pKey;
            return *this;
        }

        auto operator --()
        {
            --pKey;
            return *this;
        }

        auto operator ++(int)
        {
            auto tmp = *this;
            ++*this;
            return tmp;
        }

        auto operator --(int)
        {
            auto tmp = *this;
            --*this;
            return tmp;
        }

        bool operator ==( const _iterator& itr )
        {
            return pKey == itr.pKey;
        }

        bool operator !=( const _iterator& itr )
        {
            return pKey != itr.pKey;
        }

        #pragma endregion
    };

    #pragma region STD Iterator Functions

    using iterator = Iterator;
    using const_iterator = ConstIterator;

    iterator begin()
    {
        return Iterator( this, _pKeys );
    }

    iterator end()
    {
        return Iterator( this, _pKeys + _count );
    }

    const_iterator cbegin() const
    {
        return ConstIterator( this, _pKeys );
    }

    const_iterator cend() const
    {
        return ConstIterator( this, _pKeys + _count );
    }

    const_iterator begin() const
    {
        return cbegin();
    }

    const_iterator end() const
    {
        return cend();
    }

    #pragma endregion

    #pragma endregion

    #undef _pKeys
    #undef _pValues

};

namespace std
{
    template< typename K, typename V >
    void swap( Dictionary< K, V >& a, Dictionary< K, V >& b )
    {
        std::swap( a._pData, b._pData );
        std::swap( a._capacity, b._capacity );
        std::swap( a._count, b._count );
    }
}
