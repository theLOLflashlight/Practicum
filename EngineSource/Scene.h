// Andrew Meckling
#pragma once
#include "TextureManager.h"
#include "glhelp.h"

#include <SDL/SDL.h>

class SceneManager;

// Abstract base class for scenes used by the scene manager.
class Scene
{
public:
    friend class SceneManager;

	SceneManager* pSceneManager;
    SDL_Window* pWindow;

    Framebuffer framebuffer;
    int width;
    int height;

    unsigned prevTicks;
    unsigned ticksDiff;
    bool expired = true;

    // Constructs a scene with given dimensions and optional framebuffer attachments.
    Scene( SDL_Window* pWindow, Framebuffer::Attachments atch = Framebuffer::COLOR )
        : pWindow( (SDL_GetWindowSize( pWindow, &width, &height ), pWindow) )
        , framebuffer( width, height, atch )
    {
    }

    // GL cleanup.
    virtual ~Scene()
    {
        framebuffer.glDelete();
    }

    // Returns the number of ticks between the last 2 calls to update(unsigned).
    unsigned ticksSinceLastFrame() const
    {
        return ticksDiff;
    }

    // Call this BEFORE <derived class>::update(unsigned)
    virtual void update( unsigned ticks )
    {
        ticksDiff = ticks - prevTicks;
        prevTicks = ticks;
    }

    // Call this BEFORE <derived class>::draw()
    virtual void draw() = 0
    {
        glBindFramebuffer( GL_FRAMEBUFFER, framebuffer.frame );
        glViewport( 0, 0, width, height );
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    }

    // Called once when added to a scene manager.
    virtual void start( unsigned ticks )
    {
        prevTicks = ticks;
        expired = false;
    };

    // Called whenever this scene becomes the top scene in a scene manager.
    virtual void resume( unsigned ticks )
    {
        prevTicks = ticks;
    }

    // Called whenever this scene is no longer the top scene in a scene manager.
    virtual void pause( unsigned ticks )
    {
        prevTicks = ticks;
    }

    // Called once when removed from a scene manager.
    virtual void stop( unsigned ticks )
    {
        expired = true;
    }

};
