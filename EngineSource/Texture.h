#pragma once

#include "TextureManager.h"

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "Util.h"
#include "glhelp.h"

struct Texture
{
    int       textureUnit;
    Rect      spriteView;
    glm::vec2 size;
    glm::vec4 color { 1, 1, 1, 1 };

    Texture() = default;

    Texture( int unit, Rect view, glm::vec2 size = { 2048, 2048 },
             glm::vec4 color = { 1, 1, 1, 1 } )
        : textureUnit( unit )
        , spriteView( view )
        , size( size )
        , color( color )
    {
    }
};
