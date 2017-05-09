#pragma once

#include "glhelp.h"
#include "Drawable.h"
#include "Mesh.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

// Provides access to a single glProgram and its uniforms.
class RenderContext
{
protected:

    const GLuint programId;
    const GLuint uMatrix, uSprite, uSize, uTexture, uColor;

public:

    // Camera center position. Call useMatrix after changing this value.
    glm::vec2 camPos { 0, 0 };
    glm::vec2 camArea = { 1, 1 };

private:

    glm::mat4 _matrix;

    Mesh _rect = Mesh::make_rect( 1, 1 );

public:

    glm::mat4 computeMatrix() const
    {
        return glm::ortho(
            camPos.x,
            camPos.x + camArea.x,
            -camPos.y - camArea.y,
            -camPos.y );
    }

    // Constructs a RenderContext with an optional camera position.
    RenderContext( /*const char* vert, const char* frag*/ )
        : programId( load_shaders( "Shaders/vert.glsl", "Shaders/frag.glsl" ) )
        , uMatrix( glGetUniformLocation( programId, "uMatrix" ) )
        , uSprite( glGetUniformLocation( programId, "uSprite" ) )
        , uSize( glGetUniformLocation( programId, "uSize" ) )
        , uTexture( glGetUniformLocation( programId, "uTexture" ) )
        , uColor( glGetUniformLocation( programId, "uColor" ) )
    {
        // Init uniform defaults.
        beginFrame();
        useMatrix( computeMatrix() ); // Identity.
        useSprite( { 0, 0, 2048, 2048 } ); // Identity.
        useSize( { 2048, 2048 } ); // Identity.
        useTextureUnit( 0 );
        useColor( { 1, 1, 1, 1 } ); // White, full opacity.
        endFrame();
    }

    // GL cleanup.
    ~RenderContext()
    {
        glDeleteProgram( programId );
    }

    // Call this before calling other RenderContext methods.
    void beginFrame()
    {
        glUseProgram( programId );
        _matrix = computeMatrix();
    }

    // Call this after calling the other RenderContext methods.
    void endFrame()
    {
        glBindVertexArray( 0 );
        glUseProgram( 0 );
    }

    // Sets the projection matrix uniform after applying the camera translation.
    void useMatrix( const glm::mat4& mtx )
    {
        glUniform( uMatrix, mtx );
    }

    void useTranslation( const glm::vec2& off )
    {
        using namespace glm;
        useMatrix( translate( _matrix, vec3( off, 0 ) ) );
    }

    void useTranslationScale( const glm::vec2& off, const glm::vec2& scale )
    {
        using namespace glm;
        mat4 mtx = translate( _matrix, vec3( off, 0 ) );
        mtx = glm::scale( mtx, vec3( scale, 0 ) );
        useMatrix( mtx );
    }

    // Sets the sprite atlas uniform.
    void useSprite( const glm::vec4& vec )
    {
        glUniform( uSprite, vec );
    }

    void useSize( const glm::vec2& vec )
    {
        glUniform( uSize, vec );
    }

    // Sets the texture unit uniform.
    void useTextureUnit( int unit )
    {
        glUniform( uTexture, unit );
    }

    // Sets the color uniform.
    void useColor( const glm::vec4& vec )
    {
        glUniform( uColor, vec );
    }

    // Binds and renders a Drawable.
    void render( const Drawable& drawable )
    {
        glBindVertexArray( drawable.vertexArray );
        glDrawArrays( drawable.mode, 0, drawable.count );
    }

    // Binds and renders a Drawable.
    void render( const Mesh& mesh )
    {
        glBindVertexArray( mesh.vertexArray );
        glDrawArrays( mesh.mode, 0, mesh.count );
    }

    // Renders a rectangle. pos is top-left corner.
    void fillRect( const glm::vec2& pos, const glm::vec2& size )
    {
        useTranslationScale( pos, size );
        render( _rect );
    }
};
