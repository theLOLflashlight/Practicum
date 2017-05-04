// Unfinished
#pragma once

#include "Util.h"

#include <array>
#include <vector>

#include <glm/glm.hpp>

template< typename T, size_t Depth >
class QuadTree;

struct AABB
{
    glm::vec2 center;
    glm::vec2 halfSize;

    AABB( glm::vec2 center, glm::vec2 halfSize )
        : center( center )
        , halfSize( halfSize )
    {
    }

    bool contains( glm::vec2 a ) const
    {
        return (a.x < center.x + halfSize.x && a.x > center.x - halfSize.x)
            && (a.y < center.y + halfSize.y && a.y > center.y - halfSize.y);
    }

    bool intersects( const AABB& b ) const
    {
        return ((center.x + halfSize.x > b.center.x - b.halfSize.x)
            || (center.x - halfSize.x < b.center.x + b.halfSize.x))
            && ((center.y + halfSize.y > b.center.y - b.halfSize.y)
            || (center.y - halfSize.y < b.center.y + b.halfSize.y));
    }
};

template< typename T >
struct QuadTreeData
{
    glm::vec2 pos;
    T         load;
};

template< typename T >
class QuadTree< T, 0 >
    : public AABB
{
public:

    using ValueType = T;
    using DataType = QuadTreeData< ValueType >;
    static constexpr size_t DEPTH = 0;
    using ObjStorage = std::vector< DataType >;

private:
    
    ObjStorage _objs;

public:

    QuadTree( glm::vec2 center, glm::vec2 halfSize )
        : AABB( center, halfSize )
    {
    }

    bool insert( glm::vec2 pos, const ValueType& val )
    {
        if ( !contains( pos ) )
            return false;

        _objs.push_back( { pos, val } );
        return true;
    }

    auto query( const AABB& range )
    {
        std::vector< T > inRange;

        if ( !intersects( range ) )
            return inRange;

        for ( DataType& o : _objs )
            if ( range.contains( o.pos ) )
                inRange.push_back( o.load );

        return inRange;
    }

    bool empty() const
    {
        return _objs.empty();
    }
};

template< typename T, size_t Depth >
class QuadTree
    : public AABB
{
public:

    using ValueType = T;
    using DataType = QuadTreeData< ValueType >;
    static constexpr size_t DEPTH = Depth;

    using ObjStorage = std::vector< DataType >;
    using Quad = QuadTree< ValueType, DEPTH - 1 >;
    using Quads = std::array< Quad, 4 >;

private:

    ObjStorage _objs;
    Quads      _quads;

public:

    QuadTree( glm::vec2 qCenter, glm::vec2 qSize )
        : AABB( qCenter, qSize )
        , _quads {
            Quad { (qSize /= 2, qCenter - qSize), qSize }, // <- Comma operator.
            Quad { { qCenter.x + qSize.x, qCenter.y - qSize.y }, qSize },
            Quad { { qCenter.x - qSize.x, qCenter.y + qSize.y }, qSize },
            Quad { qCenter + qSize, qSize },
        }
    {
    }

    bool insert( glm::vec2 pos, const ValueType& val )
    {
        if ( !contains( pos ) )
            return false;

        //if ( !_objs.empty() )
        for ( Quad& quad : _quads )
            if ( quad.insert( pos, val ) )
                return true;

        _objs.push_back( { pos, val } );
        return true;
    }

    auto query( const AABB& range )
    {
        std::vector< T > inRange;

        if ( !intersects( range ) )
            return inRange;

        for ( DataType& o : _objs )
            if ( range.contains( o.pos ) )
                inRange.push_back( o.load );

        for ( Quad& quad : _quads )
            array_cat( inRange, quad.query( range ) );

        return inRange;
    }

    bool empty() const
    {
        return _objs.empty();
    }

};
