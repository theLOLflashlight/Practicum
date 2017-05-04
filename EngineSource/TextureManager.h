// Andrew Meckling
#pragma once

#include <GL/glew.h>

// Maximum number of available texture units.
// Indexes passed to odin::load_texture(...) must be smaller than this.
constexpr int NUM_TEX_UNITS = GL_MAX_TEXTURE_UNITS - GL_TEXTURE0;

// Loads a PNG file into the specified texture unit.
// Returns the loaded gl texture object on success; 0 on failure.
GLuint load_texture( int index, const char* filename );

// Loads a byte stream into the specified texture unit.
// Returns the loaded gl texture object on success; 0 on failure.
// Assumes the data uses the format GL_RGBA (4 bytes per pixel).
GLuint load_texture( int index, int width, int height, void* data );

// Loads an array-like container into the specified texture unit.
// Returns the loaded gl texture object on success; 0 on failure.
// Assumes the data uses the format GL_RGBA (4 bytes per pixel).
template< typename Array >
GLuint load_texture( int index, int width, int height, const Array& data )
{
    // Moved the functionality of this template into a non-template function
    // to reduce code bloat (from multiple instantiations of this function)
    // and potentially speed up compile time.
    return load_texture( index, width, height, (void*) std::data( data ) );
}

// Aggregates commonly used framebuffer data.
struct Framebuffer
{
    GLuint frame; // a GL_FRAMEBUFFER (Framebuffer is invalid if this is 0)
    GLuint color; // a GL_TEXTURE_2D (may be 0)
    GLuint depth; // a GL_RENDERBUFFER (may be 0)

    // Indicates which attachments a framebuffer should have.
    enum Attachments
    {
        NONE  = 0b00,
        COLOR = 0b01,
        DEPTH = 0b10,
        BOTH  = 0b11,
    };

    Framebuffer( int width, int height, Attachments attachments = BOTH );

    // Convenience method to delete gl objects managed by Framebuffer.
    void glDelete();
};
