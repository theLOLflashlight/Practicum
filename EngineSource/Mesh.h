#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <utility>

#include "glhelp.h"

struct Mesh
{
    GLenum mode = 0;
    int    count = 0;
    GLuint vertexArray = 0;
    GLuint vertexBuffer = 0;

    Mesh() = default;

    template< typename Vertex >
    Mesh( const Vertex* pVertices, int count,
          std::initializer_list< VertexAttribute > attrs,
          GLenum mode = GL_TRIANGLES )
        : mode( mode )
        , count( count )
    {
        size_t size = sizeof( Vertex ) * count;

        glGenVertexArrays( 1, &vertexArray );
        glBindVertexArray( vertexArray );

        glGenBuffers( 1, &vertexBuffer );
        glBindBuffer( GL_ARRAY_BUFFER, vertexBuffer );
        glBufferData( GL_ARRAY_BUFFER, size, pVertices, GL_STATIC_DRAW );

        int offset = 0;
        int index = 0;
        for ( VertexAttribute attr : attrs )
        {
            glEnableVertexAttribArray( index );
            glVertexAttribPointer( index, attr.size, attr.type,
                GL_FALSE, sizeof( Vertex ), (void*) offset );
            offset += attr.width;
            ++index;
        }
    }

    ~Mesh()
    {
        glDeleteVertexArrays( 1, &vertexArray );
        glDeleteBuffers( 1, &vertexBuffer );
    }

    Mesh( Mesh&& move )
        : mode( move.mode )
        , count( move.count )
        , vertexArray( move.vertexArray )
        , vertexBuffer( move.vertexBuffer )
    {
        move.vertexArray = 0;
        move.vertexBuffer = 0;
    }

    Mesh& operator =( Mesh&& move )
    {
        mode = move.mode;
        count = move.count;
        std::swap( vertexArray, move.vertexArray );
        std::swap( vertexBuffer, move.vertexBuffer );
        return *this;
    }

    Mesh copy() const
    {
        Mesh m;
        m.mode = mode;
        m.count = count;
        glGenVertexArrays( 1, &m.vertexArray );
        glBindVertexArray( m.vertexArray );
    }

    static Mesh make_rect( float width, float height, glm::vec2 tex = { 1, 1 } )
    {
        float x = tex.x;
        float y = tex.y;

        float vertices[][4] = {
            { -width / 2, +height / 2,   0, 0 },
            { -width / 2, -height / 2,   0, y },
            { +width / 2, -height / 2,   x, y },
            { +width / 2, +height / 2,   x, 0 },
        };
            
        return Mesh( vertices, 4, {
            attribute< float >( 2 ),
            attribute< float >( 2 )
        }, GL_TRIANGLE_FAN );
    }

    static Mesh make_hex( float radius )
    {
        constexpr float A = 0.86602540378;
        float r = radius;

        float vertices[][4] = {
            { 0,      +r,         0.5, 0 },
            { -A * r, +0.5 * r,   0, 0.25 },
            { +A * r, +0.5 * r,   1, 0.25 },
            { -A * r, -0.5 * r,   0, 0.75 },
            { +A * r, -0.5 * r,   1, 0.75 },
            { 0,      -r,         0.5, 1 },
        };

        return Mesh( vertices, 6, {
            attribute< float >( 2 ),
            attribute< float >( 2 )
        }, GL_TRIANGLE_STRIP );
    }

    static Mesh make_right_tri( float width, float height )
    {
        float vertices[][4] = {
            { -width / 2, -height / 2,  0, 1 },
            { -width / 2, +height / 2,  0, 0 },
            { +width / 2, -height / 2,  1, 1 },
        };

        return Mesh( vertices, 3, {
            attribute< float >( 2 ),
            attribute< float >( 2 )
        } );
    }

    static Mesh make_eq_tri( float length )
    {
        const float h = float( 0.86602540378 * length ); // ?3/2 * a

        float vertices[][4] = {
            { 0, 2 * h / 3,         0.5, 0 },
            { +length / 2, -h / 3,  1, 1 },
            { -length / 2, -h / 3,  0, 1 },
        };

        return Mesh( vertices, 3, {
            attribute< float >( 2 ),
            attribute< float >( 2 )
        } );
    }

};
