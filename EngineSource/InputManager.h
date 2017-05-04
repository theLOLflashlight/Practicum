// Andrew Meckling
#pragma once

#include <SDL/SDL.h>
#include <glm/glm.hpp>

#include <bitset>
#include <array>

#include "Dictionary.h"
#include "Util.h"

#include "ControllerManager.h"

// Sorted array of SDL_Keycode mappings KEYS[ n ] -> SDL_Keycode
constexpr SDL_Keycode KEYS[] = {
    SDLK_UNKNOWN,
    SDLK_BACKSPACE,
    SDLK_TAB,
    SDLK_RETURN,
    SDLK_ESCAPE,
    SDLK_SPACE,
    SDLK_EXCLAIM,
    SDLK_QUOTEDBL,
    SDLK_HASH,
    SDLK_DOLLAR,
    SDLK_PERCENT,
    SDLK_AMPERSAND,
    SDLK_QUOTE,
    SDLK_LEFTPAREN,
    SDLK_RIGHTPAREN,
    SDLK_ASTERISK,
    SDLK_PLUS,
    SDLK_COMMA,
    SDLK_MINUS,
    SDLK_PERIOD,
    SDLK_SLASH,
    SDLK_0,
    SDLK_1,
    SDLK_2,
    SDLK_3,
    SDLK_4,
    SDLK_5,
    SDLK_6,
    SDLK_7,
    SDLK_8,
    SDLK_9,
    SDLK_COLON,
    SDLK_SEMICOLON,
    SDLK_LESS,
    SDLK_EQUALS,
    SDLK_GREATER,
    SDLK_QUESTION,
    SDLK_AT,
    SDLK_LEFTBRACKET,
    SDLK_BACKSLASH,
    SDLK_RIGHTBRACKET,
    SDLK_CARET,
    SDLK_UNDERSCORE,
    SDLK_BACKQUOTE,
    SDLK_a,
    SDLK_b,
    SDLK_c,
    SDLK_d,
    SDLK_e,
    SDLK_f,
    SDLK_g,
    SDLK_h,
    SDLK_i,
    SDLK_j,
    SDLK_k,
    SDLK_l,
    SDLK_m,
    SDLK_n,
    SDLK_o,
    SDLK_p,
    SDLK_q,
    SDLK_r,
    SDLK_s,
    SDLK_t,
    SDLK_u,
    SDLK_v,
    SDLK_w,
    SDLK_x,
    SDLK_y,
    SDLK_z,
    SDLK_CAPSLOCK,
    SDLK_F1,
    SDLK_F2,
    SDLK_F3,
    SDLK_F4,
    SDLK_F5,
    SDLK_F6,
    SDLK_F7,
    SDLK_F8,
    SDLK_F9,
    SDLK_F10,
    SDLK_F11,
    SDLK_F12,
    SDLK_PRINTSCREEN,
    SDLK_SCROLLLOCK,
    SDLK_PAUSE,
    SDLK_INSERT,
    SDLK_HOME,
    SDLK_PAGEUP,
    SDLK_DELETE,
    SDLK_END,
    SDLK_PAGEDOWN,
    SDLK_RIGHT,
    SDLK_LEFT,
    SDLK_DOWN,
    SDLK_UP,
    SDLK_NUMLOCKCLEAR,
    SDLK_KP_DIVIDE,
    SDLK_KP_MULTIPLY,
    SDLK_KP_MINUS,
    SDLK_KP_PLUS,
    SDLK_KP_ENTER, 
    SDLK_KP_1,
    SDLK_KP_2,
    SDLK_KP_3,
    SDLK_KP_4,
    SDLK_KP_5,
    SDLK_KP_6,
    SDLK_KP_7,
    SDLK_KP_8,
    SDLK_KP_9,
    SDLK_KP_0,
    SDLK_KP_PERIOD,
    SDLK_LCTRL,
    SDLK_LSHIFT,
    SDLK_LALT,
    SDLK_LGUI,
    SDLK_RCTRL,
    SDLK_RSHIFT,
    SDLK_RALT,
    SDLK_RGUI,
};

enum MouseButton
{
    LEFT_BUTTON, MIDDLE_BUTTON, RIGHT_BUTTON, X1_BUTTON, X2_BUTTON
};

struct MouseState
{
    int x, y, wheel;
    std::bitset< 5 > buttons;
};

// Manages input events. Call pollEvents to update the stored state 
// of the connected input devices (keyboard, controllers, etc.).
class InputManager
{
public:

    static constexpr size_t NUM_KEYS = sizeof( KEYS ) / sizeof( SDL_Keycode );

private:

    MouseState _currMouse;
    MouseState _prevMouse;
    std::bitset< NUM_KEYS > _currKeys;
    std::bitset< NUM_KEYS > _prevKeys;
    bool _mouseHover;

public:

    ControllerManager gamepads;

    // Maps an sdl keycode to an index in the range [0, NUM_KEYS).
    // Returns 0 if the keycode is not recognized.
    static size_t key_index( SDL_Keycode key )
    {
        auto itr = binary_search( KEYS, key );

        return itr == std::end( KEYS ) ? 0
            : std::distance( std::begin( KEYS ), itr );
    }

    // Calls SDL_PollEvent repeatedly to empty the event queue.
    // Updates the stored state of the connected input devices.
    void pollEvents()
    {
        return pollEvents( [](){} );
    }

    // Calls SDL_PollEvent repeatedly to empty the event queue.
    // Updates the stored state of the connected input devices.
    // Invokes the callback object if the SDL_QUIT event is detected.
    template< typename Callback >
    void pollEvents( Callback&& callback )
    {
        _prevMouse = _currMouse;
        _prevKeys = _currKeys;
        std::copy( gamepads.currButtons.begin(),
                   gamepads.currButtons.end(),
                   gamepads.prevButtons.begin() );

        //gamepads.prevLeftAxis = gamepads.leftAxis;
        //gamepads.prevRightAxis = gamepads.rightAxis;
        gamepads.prevTriggerAxis = gamepads.triggerAxis;
        _currMouse.wheel = 0;

        SDL_Event event;
        while ( SDL_PollEvent( &event ) )
        {
            switch ( event.type )
            {
            case SDL_WINDOWEVENT:
            {
                switch ( event.window.event )
                {
                case SDL_WINDOWEVENT_SHOWN:
                    //SDL_Log("Window %d shown", event.window.windowID);
                    break;
                case SDL_WINDOWEVENT_HIDDEN:
                    //SDL_Log("Window %d hidden", event.window.windowID);
                    break;
                case SDL_WINDOWEVENT_EXPOSED:
                    //SDL_Log("Window %d exposed", event.window.windowID);
                    break;
                case SDL_WINDOWEVENT_MOVED:
                    //SDL_Log("Window %d moved to %d,%d",
                    //         event.window.windowID, event.window.data1,
                    //         event.window.data2);
                    break;
                case SDL_WINDOWEVENT_RESIZED:
                    //SDL_Log("Window %d resized to %dx%d",
                    //         event.window.windowID, event.window.data1,
                    //         event.window.data2);
                    break;
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                    //SDL_Log("Window %d size changed to %dx%d",
                    //         event.window.windowID, event.window.data1,
                    //         event.window.data2);
                    break;
                case SDL_WINDOWEVENT_MINIMIZED:
                    //SDL_Log("Window %d minimized", event.window.windowID);
                    break;
                case SDL_WINDOWEVENT_MAXIMIZED:
                    //SDL_Log("Window %d maximized", event.window.windowID);
                    break;
                case SDL_WINDOWEVENT_RESTORED:
                    //SDL_Log("Window %d restored", event.window.windowID);
                    break;
                case SDL_WINDOWEVENT_ENTER:
                    //SDL_Log("Mouse entered window %d", event.window.windowID);
                    _mouseHover = true;
                    break;
                case SDL_WINDOWEVENT_LEAVE:
                    //SDL_Log("Mouse left window %d", event.window.windowID);
                    _mouseHover = false;
                    break;
                case SDL_WINDOWEVENT_FOCUS_GAINED:
                    //SDL_Log("Window %d gained keyboard focus", event.window.windowID);
                    break;
                case SDL_WINDOWEVENT_FOCUS_LOST:
                    //SDL_Log("Window %d lost keyboard focus", event.window.windowID);
                    break;
                case SDL_WINDOWEVENT_CLOSE:
                    //SDL_Log("Window %d closed", event.window.windowID);
                    break;
                    #if SDL_VERSION_ATLEAST(2, 0, 5)
                case SDL_WINDOWEVENT_TAKE_FOCUS:
                    //SDL_Log("Window %d is offered a focus", event.window.windowID);
                    break;
                case SDL_WINDOWEVENT_HIT_TEST:
                    //SDL_Log("Window %d has a special hit test", event.window.windowID);
                    break;
                    #endif
                default:
                    //SDL_Log("Window %d got unknown event %d",
                    //         event.window.windowID, event.window.event);
                    break;
                }
                break;
            }
            case SDL_MOUSEMOTION:
            {
                SDL_MouseMotionEvent& motion = event.motion;
                _currMouse.x = motion.x;
                _currMouse.y = motion.y;
                break;
            }
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
            {
                SDL_MouseButtonEvent& mbutton = event.button;
                _currMouse.buttons[ mbutton.button - 1 ] = mbutton.state == SDL_PRESSED;
                break;
            }
            case SDL_MOUSEWHEEL:
            {
                SDL_MouseWheelEvent& mWheel = event.wheel;
                _currMouse.wheel = mWheel.y;
                break;
            }
            case SDL_KEYDOWN:
                _currKeys[ key_index( event.key.keysym.sym ) ] = true;
                break;
            case SDL_KEYUP:
                _currKeys[ key_index( event.key.keysym.sym ) ] = false;
                break;
            case SDL_QUIT:
                std::invoke( callback );
                SDL_Quit();
                exit( 0 );
                break;
            
            case SDL_CONTROLLERDEVICEADDED:
            {
                int joy_index = event.cdevice.which;
                int player_index = gamepads.addController( joy_index );

                //#ifdef _DEBUG
                if ( player_index == -1 )
                    printf( "Failed to register joystick location %i\n", joy_index );
                else
                    printf( "Registered joystick id %i to player %i\n",
                            gamepads.players[ player_index ], player_index );
                //#endif
                break;
            }
            //case SDL_CONTROLLERDEVICEREMAPPED:
            case SDL_CONTROLLERDEVICEREMOVED:
            {
                SDL_JoystickID joy_id = event.cdevice.which;

                for ( auto x : gamepads.controllers )
                {
                    if ( x.value && !SDL_GameControllerGetAttached( x.value ) )
                    {
                        joy_id = x.key;
                        break;
                    }
                }

                int player_index = gamepads.removeController( joy_id );

                //#ifdef _DEBUG
                if ( player_index == -1 )
                    printf( "Failed to unregister joystick id %i\n", joy_id );
                else
                    printf( "Unregistered joystick id %i from player %i\n", joy_id, player_index );
                //#endif
                break;
            }
            case SDL_CONTROLLERAXISMOTION:
			{
                SDL_JoyAxisEvent& jaxis = event.jaxis;
				int playerNo = gamepads.findPlayerIndex( jaxis.which );

                if ( playerNo == -1 )
                    break;

                if ( jaxis.axis < 2 )
                {
                    gamepads.leftAxis[ playerNo ][ jaxis.axis ] = jaxis.value / 32767.f;
                }
                else if ( jaxis.axis < 4 )
                {
                    gamepads.rightAxis[ playerNo ][ jaxis.axis % 2 ] = jaxis.value / 32767.f;
                }
                else if ( jaxis.axis < 6 )
                {
                    gamepads.triggerAxis[ playerNo ][ jaxis.axis % 2 ] = jaxis.value / 32767.f;
                }
				break;
			}
            case SDL_CONTROLLERBUTTONDOWN:
            case SDL_CONTROLLERBUTTONUP:
            {
                SDL_ControllerButtonEvent& cbutton = event.cbutton;
                int playerNo = gamepads.findPlayerIndex( cbutton.which );

                gamepads.currButtons[ playerNo ][ cbutton.button ] = (cbutton.state == SDL_PRESSED);
                #ifdef _DEBUG
                /*printf( "P%i button %s: %i\n",
                        playerNo,
                        cbutton.state ? "down" : "up",
                        event.cbutton.button );*/
                #endif
                break;
            }
            }
        }
    }

    // Returns true if the mouse is over the window area.
    bool mouseHover() const
    {
        return _mouseHover;
    }

    // Returns the current mouse position in window coords.
    // @TODO: should return a Point or tuple of ints.
    glm::vec2 getMousePos() const
    {
        return { _currMouse.x, _currMouse.y };
    }

    // Returns true if the mouse just changed position.
    bool wasMouseMoved() const
    {
        return (_currMouse.x != _prevMouse.x) || (_currMouse.y != _prevMouse.y);
    }

    // Returns true if the mouse button just changed 
    // from being released to pressed.
    bool wasButtonPressed( MouseButton button ) const
    {
        return _currMouse.buttons[ button ] && !_prevMouse.buttons[ button ];
    }

    // Returns true if the mouse button just changed 
    // from being pressed to released.
    bool wasButtonReleased( MouseButton button ) const
    {
        return !_currMouse.buttons[ button ] && _prevMouse.buttons[ button ];
    }

    // Returns true if the mouse button is pressed.
    bool isButtonDown( MouseButton button ) const
    {
        return _currMouse.buttons[ button ];
    }

    // Returns true if a specific key just changed from 
    // being unpressed to pressed.
    bool wasKeyPressed( SDL_Keycode key ) const
    {
        return _currKeys[ key_index( key ) ] && !_prevKeys[ key_index( key ) ];
    }

    // Returns true if a specific key just changed from 
    // being pressed to unpressed.
    bool wasKeyReleased( SDL_Keycode key ) const
    {
        return !_currKeys[ key_index( key ) ] && _prevKeys[ key_index( key ) ];
    }

    // Returns true if a specific key is pressed.
    bool isKeyDown( SDL_Keycode key ) const
    {
        return _currKeys[ key_index( key ) ];
    }

	// Returns a negative or positive mouse wheel value. Positive is up, negative is down
	int mouseWheelPos() const
	{
		return _currMouse.wheel;
	}

    // Returns a bitset indicating if the supplied keys were pressed.
    template< typename... Keys >
    auto wereKeysPressed( Keys... keys ) const
        -> std::bitset< sizeof...(Keys) >
    {
        std::bitset< sizeof...(Keys) > bitset;

        int i = 0;
        for ( SDL_Keycode key : { keys... } )
            bitset[ i++ ] = wasKeyPressed( key );

        return bitset;
    }

    // Returns a bitset indicating if the supplied keys were released.
    template< typename... Keys >
    auto wereKeysReleased( Keys... keys ) const
        -> std::bitset< sizeof...(Keys) >
    {
        std::bitset< sizeof...(Keys) > bitset;

        int i = 0;
        for ( SDL_Keycode key : { keys... } )
            bitset[ i++ ] = wasKeyReleased( key );

        return bitset;
    }

    // Returns a bitset indicating if the supplied keys are down.
    template< typename... Keys >
    auto areKeysDown( Keys... keys ) const
        -> std::bitset< sizeof...(Keys) >
    {
        std::bitset< sizeof...(Keys) > bitset;

        int i = 0;
        for ( SDL_Keycode key : { keys... } )
            bitset[ i++ ] = isKeyDown( key );

        return bitset;
    }
};

class InputReceiver
{
    static InputManager& _input()
    {
        static InputManager input;
        return input;
    }

public:

    static void poll_events()
    {
        _input().pollEvents();
    }

    auto& gamepads() const
    {
        return _input().gamepads;
    }

    // Returns true if the mouse is over the window area.
    bool mouseHover() const
    {
        return _input().mouseHover();
    }

    // Returns the current mouse position in window coords.
    // @TODO: should return a Point or tuple of ints.
    auto getMousePos() const
    {
        return _input().getMousePos();
    }

    // Returns true if the mouse just changed position.
    bool wasMouseMoved() const
    {
        return _input().wasMouseMoved();
    }

    // Returns true if the mouse button just changed 
    // from being released to pressed.
    bool wasButtonPressed( MouseButton button ) const
    {
        return _input().wasButtonPressed( button );
    }

    // Returns true if the mouse button just changed 
    // from being pressed to released.
    bool wasButtonReleased( MouseButton button ) const
    {
        return _input().wasButtonReleased( button );
    }

    // Returns true if the mouse button is pressed.
    bool isButtonDown( MouseButton button ) const
    {
        return _input().isButtonDown( button );
    }

    // Returns true if a specific key just changed from 
    // being unpressed to pressed.
    bool wasKeyPressed( SDL_Keycode key ) const
    {
        return _input().wasKeyPressed( key );
    }

    // Returns true if a specific key just changed from 
    // being pressed to unpressed.
    bool wasKeyReleased( SDL_Keycode key ) const
    {
        return _input().wasKeyReleased( key );
    }

    // Returns true if a specific key is pressed.
    bool isKeyDown( SDL_Keycode key ) const
    {
        return _input().isKeyDown( key );
    }

    // Returns a negative or positive mouse wheel value. Positive is up, negative is down
    int mouseWheelPos() const
    {
        return _input().mouseWheelPos();
    }

    // Returns a bitset indicating if the supplied keys were pressed.
    template< typename... Keys >
    auto wereKeysPressed( Keys... keys ) const
        -> std::bitset< sizeof...(Keys) >
    {
        return _input().wereKeysPressed( keys... );
    }

    // Returns a bitset indicating if the supplied keys were released.
    template< typename... Keys >
    auto wereKeysReleased( Keys... keys ) const
        -> std::bitset< sizeof...(Keys) >
    {
        return _input().wereKeysReleased( keys... );
    }

    // Returns a bitset indicating if the supplied keys are down.
    template< typename... Keys >
    auto areKeysDown( Keys... keys ) const
        -> std::bitset< sizeof...(Keys) >
    {
        return _input().areKeysDown( keys... );
    }
};
