#pragma once

#include "RenderContext.h"

// Allows rendering of text from a render context.
struct TextRenderer
{
    RenderContext* pGfx;    // RenderContext to draw with.
    int            texUnit; // Texture unit of font.
    glm::vec2      texSize; // Size of font texture.

    // Renders text. gfx.beginFrame() must have been called first.
    void render( glm::vec2          pos,
                 const std::string& text,
                 double             fontScale = 1,
                 glm::vec4          color = { 1, 1, 1, 1 } )
    {
        auto& gfx = *pGfx;
        // Set up render state for text.
        gfx.useTextureUnit( texUnit );
        gfx.useSize( texSize );
        gfx.useColor( color );

        int height = texSize.y;
        float size = height * fontScale;
        float left = pos.x += size;

        for ( char c : text )
        {
            // Handle newline.
            if ( c == '\n' )
            {
                pos = { left, pos.y - size };
                continue;
            }
            // Draw nothing for space.
            if ( c != ' ' )
            {
                // Clip top row of pixels to hide font guide.
                // '!' is the first character in the sprite font.
                gfx.useSprite( { (c - '!') * height, 1, height, height - 1 } );
                // Scale character minus top pixel row.
                gfx.fillRect( pos, { size, size - fontScale } );
            }
            // Advance to next character spot.
            pos.x += size;
        }
    }
};
