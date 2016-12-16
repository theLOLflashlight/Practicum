// Andrew Meckling
#pragma once

#include "Scene.h"

class SceneManager
{
	std::vector< Scene* > _scenes;
    std::vector< Scene* > _pendingScenes;

    void _pushScene( Scene* scene, unsigned ticks )
    {
        if ( Scene* top = topScene() )
            top->pause( ticks );

        scene->init( ticks );
        _scenes.push_back( scene );
        scene->resume( ticks );
    }

    void _popScene( unsigned ticks )
    {
        Scene* top = _scenes.back();
        top->pause( ticks );
        top->exit( ticks );
        _scenes.pop_back();
        delete top;

        if ( top = topScene() )
            top->resume( ticks );
    }

public:

    Scene* topScene()
    {
        return _scenes.empty() ? nullptr : _scenes.back();
    }

    size_t count() const
    {
        return _scenes.size();
    }

    void pushScene( Scene* scene )
    {
		scene->pSceneManager = this;
        _pendingScenes.push_back( scene );
    }

    void popScene()
    {
        _pendingScenes.push_back( nullptr );
    }

    void popScenes( size_t n )
    {
        while ( n-- > 0 )
            popScene();
    }

    void clearScenes()
    {
        int count = this->count();
        for ( Scene* scene : _pendingScenes )
            count += scene ? 1 : -1;

        for ( int i = 0; i < count; ++i )
            popScene();
    }

    void update( unsigned ticks )
    {
        //for ( auto itr = _scenes.rbegin(); itr != _scenes.rend(); ++itr )
        //    if ( (*itr)->expired )
        //        popScene();
        //    else
        //        break;

        std::vector< Scene* > tmpPendingScenes = _pendingScenes;
        _pendingScenes.clear();

        for ( Scene* scene : tmpPendingScenes )
            if ( scene == nullptr )
                _popScene( ticks );
            else
                _pushScene( scene, ticks );

        Scene* top;
        while ( (top = topScene()) && top->expired )
            _popScene( ticks );

        if ( Scene* top = topScene() )
            top->update( ticks );
    }

    void render()
    {
        if ( Scene* top = topScene() )
            top->draw();
    }

	~SceneManager()
    {
        for ( Scene* s : _pendingScenes )
            delete s;
        for ( Scene* s : _scenes )
            delete s;
	}

};
