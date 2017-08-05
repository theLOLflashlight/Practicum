// Andrew Meckling
#pragma once

#include "Scene.h"

// Manages a collection of scenes in a stack. Scenes held by this class 
// are not "owned" by it; each scene must manage its own lifetime outside
// of this class.
class SceneManager
{
private:

	std::vector< Scene* > _scenes;
    std::vector< Scene* > _pendingScenes;

    // Immediately pushes a scene onto the stack.
    void _pushScene( Scene* scene, unsigned ticks )
    {
        if ( Scene* top = topScene() )
            top->pause( ticks );

        scene->start( ticks );
        scene->pSceneManager = this;

        _scenes.push_back( scene );
        scene->resume( ticks );
    }

    // Immediately pops a scene from the stack.
    void _popScene( unsigned ticks )
    {
        assert( !_scenes.empty() );
        Scene* top = _scenes.back();

        top->pause( ticks );
        top->pSceneManager = nullptr;

        _scenes.pop_back();
        top->stop( ticks );

        if ( (top = topScene()) )
            top->resume( ticks );
    }

public:

    // Gets the top scene or nullptr if there are no scenes.
    Scene* topScene()
    {
        return _scenes.empty() ? nullptr : _scenes.back();
    }

    // Gets the number of scenes.
    size_t count() const
    {
        return _scenes.size();
    }

    // Queues a scene push transaction.
    void pushScene( Scene* scene )
    {
        _pendingScenes.push_back( scene );
    }

    // Queues a scene pop transaction.
    void popScene()
    {
        _pendingScenes.push_back( nullptr );
    }

    // Queues multiple scene pop transactions.
    void popScenes( size_t n )
    {
        while ( n-- > 0 )
            popScene();
    }

    // Queues the removal of all scenes from the stack.
    void clearScenes()
    {
        int count = this->count();
        for ( Scene* scene : _pendingScenes )
            count += scene ? 1 : -1;

        for ( int i = 0; i < count; ++i )
            popScene();
    }

    // Applies pending stack transactions then updates the top scene.
    void update( unsigned ticks )
    {
        for ( Scene* scene : _pendingScenes )
            if ( scene == nullptr )
                _popScene( ticks );
            else
                _pushScene( scene, ticks );

        _pendingScenes.clear();

        Scene* top;
        while ( (top = topScene()) && top->expired )
            _popScene( ticks );

        if ( Scene* top = topScene() )
            top->update( ticks );
    }

    // Draws the top scene to its framebuffer then blits the framebuffer
    // to the default framebuffer with supplied dimensions.
    void render( size_t width, size_t height )
    {
        if ( Scene* top = topScene() )
        {
            top->draw();

            glBindFramebuffer( GL_FRAMEBUFFER, 0 );
            glViewport( 0, 0, width, height );
            glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

            glBindFramebuffer( GL_READ_FRAMEBUFFER, top->framebuffer.frame );
            glReadBuffer( GL_COLOR_ATTACHMENT0 );

            glBindFramebuffer( GL_DRAW_FRAMEBUFFER, 0 );
            GLenum drawbuf = GL_COLOR_ATTACHMENT0;
            glDrawBuffers( 1, &drawbuf );

            glBlitFramebuffer(
                0, 0, top->width, top->height,
                0, 0, width, height,
                GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
                GL_NEAREST );
        }
    }

};
