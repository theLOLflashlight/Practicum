#pragma once

#include "TextureManager.h"

#include <vector>
#include <array>

class GlTextureManager
{
private:

    std::vector< GLuint > _textures;
    std::array< GLuint, NUM_TEX_UNITS > _units;

public:

    GlTextureManager()
    {
        GLuint defTex = load_texture< GLubyte[ 4 ] >( 0, 1, 1, { 0xff, 0xff, 0xff, 0xff } );
        _textures.push_back( defTex );
        _units[ 0 ] = defTex;
    }

    ~GlTextureManager()
    {
        glDeleteTextures( _textures.size(), _textures.data() );
    }


};