// Unfinished
#pragma once

#include "LocalVector.h"

#include <array>
#include <memory>
#include <vector>

#include <glm/glm.hpp>

struct AABB
{
    glm::vec2 center;
    glm::vec2 halfSize;

    AABB( glm::vec2 center, glm::vec2 halfSize )
        : center( center )
        , halfSize( halfSize )
    {
    }

    explicit AABB( glm::vec2 center )
        : AABB( center, { 0, 0 } )
    {
    }

    explicit AABB( Rect rect )
        : AABB(
            rect.x + rect.width / 2,
            rect.y + rect.height / 2,
            rect.width / 2,
            rect.height / 2 )
    {
    }

    AABB( float cx, float cy, float hsx, float hsy )
        : AABB( { cx, cy }, { hsx, hsy } )
    {
    }

    float left() const
    {
        return center.x - halfSize.x;
    }

    float right() const
    {
        return center.x + halfSize.x;
    }

    float top() const
    {
        return center.y + halfSize.y;
    }

    float bottom() const
    {
        return center.y - halfSize.y;
    }

    bool contains( glm::vec2 a ) const
    {
        return (a.x < right() && a.x > left())
            && (a.y < top() && a.y > bottom());
    }

    bool intersects( const AABB& b ) const
    {
        return right() > b.left() && left() < b.right()
            && top() > b.bottom() && bottom() < b.top();
    }

    bool contains( const AABB& b ) const
    {
        return intersects( b )
            && b.right() <= right()
            && b.left() >= left()
            && b.top() <= top()
            && b.bottom() >= bottom();
    }
};

template< typename T >
struct QuadTreeData : AABB
{
    T load;

    QuadTreeData() = default;
    QuadTreeData( const AABB& aabb, T value )
        : AABB( aabb ), load( std::move( value ) )
    {
    }

    operator T&()
    {
        return load;
    }
};

template< typename T >
class QuadTree
    : private AABB
{
public:

    using ValueType = T;
    using DataType = QuadTreeData< ValueType >;
    using QueryResult = std::vector< std::reference_wrapper< DataType > >;
    
    using ObjStorage = std::vector< DataType >;
    using QuadPtr = std::unique_ptr< QuadTree >;
    using Quads = std::array< QuadPtr, 4 >;

    using AABB::contains;
    using AABB::intersects;

    QuadTree( const AABB& aabb )
        : AABB( aabb )
    {
    }

    QuadTree( glm::vec2 center, glm::vec2 halfSize )
        : AABB( center, halfSize )
    {
    }

    const AABB& aabb() const
    {
        return static_cast< const AABB& >( *this );
    }

private:
    
    ObjStorage _objs;
    Quads      _quads;
     
    void _subdivide()
    {
        auto qSize = halfSize * 0.5f;
        auto newQuad = [qSize]( glm::vec2 center ) {
            return std::make_unique< QuadTree >( center, qSize );
        };

        _quads[ 0 ] = newQuad( center + qSize );
        _quads[ 1 ] = newQuad( { center.x - qSize.x, center.y + qSize.y } );
        _quads[ 2 ] = newQuad( center - qSize );
        _quads[ 3 ] = newQuad( { center.x + qSize.x, center.y - qSize.y } );
    }

    QuadTree* _getDest( const AABB& a )
    {
        if ( isSubdivided() )
            for ( QuadPtr& pQuad : _quads )
                if ( pQuad->contains( a ) )
                    if ( QuadTree* pTree = pQuad->_getDest( a ) )
                        return pTree;

        return contains( a ) ? this : nullptr;
    }

    void _subQuery( const AABB& aabb, QueryResult& result )
    {
        if ( !intersects( aabb ) )
            return;

        for ( DataType& data : _objs )
            if ( &aabb != (AABB*) &data && aabb.intersects( data ) )
                result.push_back( data );

        if ( isSubdivided() )
            for ( QuadPtr& pQuad : _quads )
                pQuad->_subQuery( aabb, result );
    }

public:

    template< typename Func >
    void forEach( Func&& func, size_t level = 0 )
    {
        for ( auto& obj : _objs )
            std::invoke( func, obj, level );

        if ( isSubdivided() )
            for ( QuadPtr& pTree : _quads )
                pTree->forEach( func, level + 1 );
    }

    bool isSubdivided() const
    {
        return _quads[ 0 ] != nullptr;
    }

    bool empty() const
    {
        if ( !_objs.empty() )
            return false;

        if ( isSubdivided() )
            for ( QuadPtr& pTree : const_cast< Quads& >( _quads ) )
                if ( !pTree->empty() )
                    return false;

        return true;
    }

    bool insert( const AABB& aabb, const ValueType& val )
    {
        if ( !contains( aabb ) )
            return false;

        QuadTree* pTree = _getDest( aabb );

        if ( pTree == this )
        {
            if ( !isSubdivided() )
            {
                _subdivide();
                pTree = _getDest( aabb );
            }
            else
            {
                _objs.emplace_back( aabb, val );
                return true;
            }
        }

        return pTree->insert( aabb, val );
    }

    bool test( const AABB& aabb )
    {
        if ( !intersects( aabb ) )
            return false;

        for ( DataType& data : _objs )
            if ( &aabb != (AABB*) &data && aabb.intersects( data ) )
                return true;

        if ( isSubdivided() )
            for ( QuadPtr& pQuad : _quads )
                if ( pQuad->test( aabb ) )
                    return true;

        return false;
    }

    QueryResult query( const AABB& aabb )
    {
        QueryResult result;
        _subQuery( aabb, result );
        return result;
    }

    void update( QuadTree* parent = nullptr )
    {
        if ( isSubdivided() )
            for ( QuadPtr& pQuad : _quads )
                pQuad->update( parent ? parent : this );

        auto tmpObjs = std::move( _objs );
        _objs.clear();

        for ( DataType& data : tmpObjs )
        {
            if ( !contains( data ) )
            {
                if ( parent )
                    parent->insert( data, data.load );
            }
            else
            {
                insert( data, data.load );
            }
        }

        if ( empty() )
            clear();
    }

    void clear()
    {
        _objs.clear();

        if ( isSubdivided() )
            for ( QuadPtr& pQuad : _quads )
                pQuad = nullptr; // Frees subtree.
    }

    void clear( const AABB& range )
    {
        if ( !intersects( range ) )
            return;

        ObjStorage tmp = move( _objs );
        _objs.clear();

        for ( DataType& data : tmp )
            if ( !range.intersects( data ) )
                _objs.push_back( data );

        if ( isSubdivided() )
            for ( QuadPtr& pQuad : _quads )
                pQuad->clear( range );
    }

};
