// Andrew Meckling

#define SDL_MAIN_HANDLED

#include "LevelScene.h"

#include "EditScene.h"

#include "SceneManager.h"
#include "InputManager.h"
#include "AudioEngine.h"

#include <SDL/SDL.h>
#include <GL/glew.h>
#include <glm/glm.hpp>

#include <cstdio>
#include <ctime>

#include "Rubiks.h"
#include "SparseBucketArray.h"

SDL_Window* create_window( const char* title, int width, int height );

namespace _game
{
    bool running = true;

    const double target_frame_time = 1 / 60.0;
    const int target_frame_time_ms = target_frame_time * 1000;
}

int main( int argc, char* argv[] )
{
    srand( (unsigned) time( 0 ) );
    printf( "Hello Practicum!\n" );

    SparseBucketArray< int, 16 > bucketarr;

    bucketarr[ 0 ] = 1;
    bucketarr[ 16 ] = 2;
    bucketarr[ 64 ] = 3;
    bucketarr[ 32 ] = 4;
    bucketarr[ 1 ] = 5;
    bucketarr[ 2 ] = 6;
    bucketarr.remove( 16 );
    bucketarr[ 16 ] = 15;

    //bucketarr.forEach( []( auto i ) {
    //    printf( "%i ", i );
    //} );

    {
        using namespace moves;

        sizeof( Facelet );
        sizeof( EdgeCubie );
        sizeof( CornerCubie );
        sizeof( std::array< Facelet, 54 > );
        sizeof( std::array< EdgeCubie, 12 > );
        sizeof( std::array< CornerCubie, 8 > );
        sizeof( Permutation );

        Permutation perm( Ri );

        auto p = U * R * perm;

        int i = 5;
    }

    if ( SDL_Init( SDL_INIT_EVERYTHING ) != 0 )
    {
        printf( "SDL_Init failed: %s.\n", SDL_GetError() );
        SDL_Quit();
    }

    SDL_DisplayMode DM;
    SDL_GetCurrentDisplayMode( 0, &DM );
    const int screen_width = DM.w;
    const int screen_height = DM.h;

    const int width = 32 * LevelScene::FACELET_W * LevelScene::FACELET_X;
    const int height = 32 * LevelScene::FACELET_H * LevelScene::FACELET_Y;

    SDL_Window* pWindow = create_window( "Rynth", width, height );

    SceneManager sceneManager;
    AudioEngine::init();

    LevelScene levelScene( pWindow );
    EditScene editScene( pWindow );

    //sceneManager.pushScene( &levelScene );
    sceneManager.pushScene( &editScene );

    // Game loop.
    while ( _game::running )
    {
        int frameStart = SDL_GetTicks();

        // Tick.
        {
            InputReceiver::poll_events();

            sceneManager.update( frameStart );
            AudioEngine::update();

            sceneManager.render( width, height );
        }

        // Cap framerate at 60fps
        int frameEnd = SDL_GetTicks();
        int frameTime_ms = (frameEnd - frameStart);

        if ( frameTime_ms <= _game::target_frame_time_ms )
            SDL_Delay( _game::target_frame_time_ms - frameTime_ms );
        else
            printf( "Frame %ims slow\n", frameTime_ms - _game::target_frame_time_ms );

        SDL_GL_SwapWindow( pWindow );
    }

    AudioEngine::shutdown();
    SDL_DestroyWindow( pWindow );

    return 0;
}

SDL_Window* create_window( const char* title, int width, int height )
{
    // Request minimum version for compatibility.
    if ( SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 4 ) )
        printf( "SDL_GL_SetAttribute failed: %s.\n", SDL_GetError() );
    if ( SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 0 ) )
        printf( "SDL_GL_SetAttribute failed: %s.\n", SDL_GetError() );
    if ( SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE ) )
        printf( "SDL_GL_SetAttribute failed: %s.\n", SDL_GetError() );

    // Create OpenGL window
    // SDL_sdlWindowPOS_CENTERED creates window in the center position using given width/height
    SDL_Window* _sdlWindow = SDL_CreateWindow(
        title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL );

    if ( _sdlWindow == nullptr )
    {
        printf( "SDL_Window could not be created.\n" );
        SDL_Quit();
    }

    SDL_GLContext glContext = SDL_GL_CreateContext( _sdlWindow );
    if ( glContext == nullptr )
    {
        printf( "SDL_GLContext could not be created.\n" );
        SDL_Quit();
    }

    if ( GLenum err = glewInit() )
    {   // != GLEW_OK
        fprintf( stderr, "Error: %s\n", glewGetErrorString( err ) );
        SDL_Quit();
    }

    SDL_GL_SetSwapInterval( 0 );
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    printf( "OpenGL %s\n\n", glGetString( GL_VERSION ) );

    SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "0" );

    return _sdlWindow;
}