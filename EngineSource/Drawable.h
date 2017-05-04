#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <utility>

#include "glhelp.h"

struct Drawable
{
    GLenum mode = 0;
    int    count = 0;
    GLuint vertexArray = 0;
    GLuint vertexBuffer = 0;

    // OpenGL texture unit index.
    int       texture = 0;
    glm::vec4 color = { 1, 1, 1, 1 };
	bool      visible = true;

    Drawable() = default;

    template< typename Vertex >
	Drawable( const Vertex* pVertices, int count,
              std::initializer_list< VertexAttribute > attrs,
              GLenum mode = GL_TRIANGLES,
              int textureUnit = 0 )
        : mode( mode )
        , count( count )
        , texture( textureUnit )
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

    ~Drawable()
    {
        glDeleteVertexArrays( 1, &vertexArray );
        glDeleteBuffers( 1, &vertexBuffer );
    }

    Drawable( Drawable&& move )
        : mode( move.mode )
        , count( move.count )
        , vertexArray( move.vertexArray )
        , vertexBuffer( move.vertexBuffer )
        , texture( move.texture )
        , color( move.color )
        , visible( move.visible )
    {
        move.vertexArray = 0;
        move.vertexBuffer = 0;
    }

    Drawable& operator =( Drawable&& move )
    {
        mode = move.mode;
        count = move.count;
        std::swap( vertexArray, move.vertexArray );
        std::swap( vertexBuffer, move.vertexBuffer );
        texture = move.texture;
        color = move.color;
        visible = move.visible;
        return *this;
    }

	static Drawable make_rect( float width, float height, int unit = 0, glm::vec2 tex = { 1, 1 } )
    {
        float x = tex.x;
        float y = tex.y;

        float vertices[][4] = {
            { -width / 2, +height / 2,   0, 0 },
            { -width / 2, -height / 2,   0, y },
            { +width / 2, -height / 2,   x, y },
            { +width / 2, +height / 2,   x, 0 },
        };
            
		return Drawable( vertices, 4, {
            attribute< float >( 2 ),
            attribute< float >( 2 )
        }, GL_TRIANGLE_FAN, unit );
    }

    static Drawable make_hex( float radius, int unit = 0 )
    {
        constexpr float A = 0.86602540378f;
        float r = radius;

        float vertices[][4] = {
            { 0,      +r,         0.5f, 1 },
            { -A * r, +0.5f * r,   0, 0.75f },
            { +A * r, +0.5f * r,   1, 0.75f },
            { -A * r, -0.5f * r,   0, 0.25f },
            { +A * r, -0.5f * r,   1, 0.25f },
            { 0,      -r,         0.5f, 0 },
        };

        return Drawable( vertices, 6, {
            attribute< float >( 2 ),
            attribute< float >( 2 )
        }, GL_TRIANGLE_STRIP, unit );
    }

    static Drawable make_right_tri( float width, float height )
    {
        float vertices[][4] = {
            { -width / 2, -height / 2,  0, 1 },
            { -width / 2, +height / 2,  0, 0 },
            { +width / 2, -height / 2,  1, 1 },
        };

        return Drawable( vertices, 3, {
            attribute< float >( 2 ),
            attribute< float >( 2 )
        } );
    }

    static Drawable make_eq_tri( float length )
    {
        const float h = float( 0.86602540378 * length ); // ?3/2 * a

        float vertices[][4] = {
            { 0, 2 * h / 3,         0.5f, 0 },
            { +length / 2, -h / 3,  1, 1 },
            { -length / 2, -h / 3,  0, 1 },
        };

		return Drawable( vertices, 3, {
            attribute< float >( 2 ),
            attribute< float >( 2 )
        } );
    }

};
