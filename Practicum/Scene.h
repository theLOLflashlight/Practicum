// Andrew Meckling
#pragma once
#include "TextureManager.h"
#include "glhelp.h"

class SceneManager;

class Scene
{
public:
	SceneManager* pSceneManager;

    Framebuffer framebuffer;
    int width;
    int height;

    unsigned prevTicks;
    unsigned ticksDiff;
    bool expired = true;

    virtual void init( unsigned ticks )
    {
        prevTicks = ticks;
        expired = false;
    };

    virtual void resume( unsigned ticks )
    {
        prevTicks = ticks;
    }

    virtual void pause( unsigned ticks )
    {
        prevTicks = ticks;
    }

    virtual void exit( unsigned ticks )
    {
        expired = true;
    }

    virtual ~Scene()
    {
        framebuffer.glDelete();
    }

    Scene( int width, int height )
        : framebuffer( width, height, Framebuffer::COLOR )
        , width( width )
        , height( height )
    {
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

};
