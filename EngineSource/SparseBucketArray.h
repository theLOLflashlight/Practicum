#pragma once

#include <memory>
#include <set>

#include "SparseArray.h"

template< typename T, size_t N >
struct Bucket
{
    using BucketData = SparseArray< T, N >;
    using DataPtr = std::unique_ptr< BucketData >;

    size_t  min;
    DataPtr data;

    static DataPtr new_bucket()
    {
        return std::make_unique< BucketData >();
    }
};

struct BucketComparator
{
    template< typename T, size_t N >
    constexpr bool operator ()( const Bucket< T, N >& a, const Bucket< T, N >& b ) const
    {
        return a.min < b.min;
    }
};

template< typename T, size_t BucketSize >
class SparseBucketArray
{
public:

    static constexpr size_t BUCKET_SIZE = BucketSize;

    using ValueType = T;
    using Bucket = Bucket< ValueType, BUCKET_SIZE >;

private:

    std::set< Bucket, BucketComparator > _buckets;

public:

    template< typename Func >
    void forEach( Func&& func )
    {
        for ( auto& bucket : _buckets )
            for ( auto& value : *bucket.data )
                std::invoke( forward< Func >( func ), value );
    }

    void remove( size_t idx )
    {
        size_t div = idx / BUCKET_SIZE;
        size_t mod = idx % BUCKET_SIZE;

        auto where = _buckets.find( { div * BUCKET_SIZE, nullptr } );
        if ( where != _buckets.end() )
        {
            decltype(auto) obj = (*where->data)[ mod ];
            if ( !!obj )
            {
                obj.remove();
                if ( where->data->empty() )
                    _buckets.erase( where );
            }
        }
    }

    decltype(auto) operator []( size_t idx )
    {
        size_t div = idx / BUCKET_SIZE;
        size_t mod = idx % BUCKET_SIZE;
        size_t tgt_min = div * BUCKET_SIZE;

        auto where = _buckets.find( { tgt_min, nullptr } );
        if ( where != _buckets.end() )
            return (*where->data)[ mod ];

        Bucket&& bucket { tgt_min, Bucket::new_bucket() };
        decltype(auto) obj = (*bucket.data)[ mod ];

        _buckets.insert( std::move( bucket ) );
        return obj;
    }

};
