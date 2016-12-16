// Andrew Meckling

#ifdef _DEBUG
#define SDL_MAIN_HANDLED
#endif

#include "SceneManager.h"
#include "InputManager.h"

#include <SDL/SDL.h>
#include <GL/glew.h>
#include <glm/glm.hpp>

#include <cstdio>

SDL_Window* create_window( const char* title, int width, int height );

namespace _game
{
    bool running = true;

    const double target_frame_time = 1 / 60.0;
    const int target_frame_time_ms = target_frame_time * 1000;
}

int main( int argc, char* argv[] )
{
    printf( "Hello Practicum!\n" );

    if ( SDL_Init( SDL_INIT_EVERYTHING ) != 0 )
    {
        printf( "SDL_Init failed: %s.\n", SDL_GetError() );
        SDL_Quit();
    }

    SDL_DisplayMode DM;
    SDL_GetCurrentDisplayMode( 0, &DM );
    const int screen_width = DM.w;
    const int screen_height = DM.h;

    const int width = 800;
    const int height = 600;

    SDL_Window* pWindow = create_window( "Practicum", width, height );

    InputManager inputManager;
    SceneManager sceneManager;

    // Game loop.
    while ( _game::running )
    {
        int frameStart = SDL_GetTicks();

        // Tick.
        {
            inputManager.pollEvents();

            sceneManager.update( frameStart );
            sceneManager.render();

            //audioEngine.update();

            glBindFramebuffer( GL_FRAMEBUFFER, 0 );
            glViewport( 0, 0, width, height );
            glClear( GL_COLOR_BUFFER_BIT );

            if ( Scene* top = sceneManager.topScene() )
            {
                glBindFramebuffer( GL_READ_FRAMEBUFFER, top->framebuffer.frame );
                glReadBuffer( GL_COLOR_ATTACHMENT0 );

                glBindFramebuffer( GL_DRAW_FRAMEBUFFER, 0 );
                GLenum drawbuf = GL_COLOR_ATTACHMENT0;
                glDrawBuffers( 1, &drawbuf );

                glBlitFramebuffer(
                    0, 0, top->width, top->height,
                    0, 0, width, height,
                    GL_COLOR_BUFFER_BIT,
                    GL_NEAREST );
            }
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

    SDL_DestroyWindow( pWindow );

    return 0;
}

SDL_Window* create_window( const char* title, int width, int height )
{
    // Request minimum version for compatibility.
    if ( SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 ) )
        printf( "SDL_GL_SetAttribute failed: %s.\n", SDL_GetError() );
    if ( SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 3 ) )
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