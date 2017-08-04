#pragma once

#include <SDL/SDL.h>
#include <glm/glm.hpp>

#include <bitset>
#include <array>
#include <map>

#include "HashMap.h"
#include "Dictionary.h"
#include "Util.h"

// Manages gamepads connected to the device. Does not handle its
// own input detection, merely provides convenient access to the 
// connected controllers and their states.
class ControllerManager
{
public:

    static constexpr int MAX_PLAYERS = 4;

    using PlayerIndex = int;
    using ControllerMap = HashMap< SDL_JoystickID, SDL_GameController* >;
    using PlayerList = std::array< SDL_JoystickID, MAX_PLAYERS >;

    using ButtonStates = std::array< std::bitset< SDL_CONTROLLER_BUTTON_MAX >, MAX_PLAYERS >;
    using AxisStates = std::array< glm::vec2, MAX_PLAYERS >;
    //using TiggerStates = std::array< float, MAX_PLAYERS >;
		
	ControllerMap controllers; // Maps joystick ids to game controllers.
    PlayerList    players;     // Stores the joystick id of each player.

    ButtonStates  currButtons; // Represents the current down state of each button on each controller.
    ButtonStates  prevButtons; // Represents the previous down state of each button on each controller.
	AxisStates	  leftAxis;
    AxisStates    rightAxis;
    AxisStates    triggerAxis;
    //AxisStates    prevLeftAxis;
    //AxisStates    prevRightAxis;
    AxisStates    prevTriggerAxis;

    ControllerManager()
        : controllers( { {-1, nullptr} }, MAX_PLAYERS )
        , currButtons()
        , prevButtons()
    {
        players.fill( -1 );
    }

    // Gets the player index that a given joystick id is mapped to.
    // Returns -1 if the joystick id isn't mapped to a player index.
    PlayerIndex findPlayerIndex( SDL_JoystickID id ) const
    {
        for ( PlayerIndex i = 0; i < MAX_PLAYERS; ++i )
            if ( players[ i ] == id )
                return i;
        return -1;
    }

    // Gets the controller mapped to a given player index.
    // Returns nullptr if no controller is mapped to the player index.
    SDL_GameController* getController( PlayerIndex index ) const
    {
        return controllers[ players[ index ] ];
    }

    // Activates a controller at a specific joystick index.
    // Returns the player index [0, MAX_PLAYERS) of the added
    // controller on success. Returns -1 on failure.
    PlayerIndex addController( int joyDeviceIndex )
    {
        auto p_controller = SDL_GameControllerOpen( joyDeviceIndex );
        if ( p_controller == nullptr )
            return -1; // controller could not be opened.

        // get the joystick id of the newly opened game controller.
        SDL_Joystick* p_joystick = SDL_GameControllerGetJoystick( p_controller );
        SDL_JoystickID joy_id = SDL_JoystickInstanceID( p_joystick );

        // map the joystick id to the controller.
        if ( controllers.add( joy_id, p_controller ) )
        {
            // map the joystick id to the first empty player index.
            for ( PlayerIndex i = 0; i < MAX_PLAYERS; ++i )
            {
                if ( players[ i ] == -1 )
                {
                    players[ i ] = joy_id;
                    return i;
                }
            }
            // not enough controller indexs
            controllers.remove( joy_id );
        }
        SDL_GameControllerClose( p_controller );
        return -1; // controller could not be added.
    }

    // Deactivates a controller with a specific joystick id.
    // Returns the player index [0, MAX_PLAYERS) of the controller 
    // that was removed on success. Returns -1 on failure.
    PlayerIndex removeController( SDL_JoystickID joy_id )
    {
        SDL_GameControllerClose( controllers[ joy_id ] );
        if ( controllers.remove( joy_id ) )
        {
            // unmap the joystick id from its player index.
            for ( PlayerIndex i = 0; i < MAX_PLAYERS; ++i )
            {
                if ( players[ i ] == joy_id )
                {
                    players[ i ] = -1;
                    return i;
                }
            }
        }
        return -1; // controller wasn't previously connected.
    }

	// Returns true if ANYONE pressed a given button
	bool didAnyonePressButton( SDL_GameControllerButton button ) const
    {
        for ( int i = 0; i < MAX_PLAYERS; ++i )
            if ( currButtons[ i ][ button ] )
                return true;
		return false;
	}

    // Returns true if a specific button on a specific controller just
    // changed from being unpressed to pressed.
    bool wasButtonPressed( PlayerIndex idx, SDL_GameControllerButton button ) const
    {
        return currButtons[ idx ][ button ] && !prevButtons[ idx ][ button ];
    }

    // Returns true if a specific button on a specific controller just
    // changed from being pressed to unpressed.
    bool wasButtonReleased( PlayerIndex idx, SDL_GameControllerButton button ) const
    {
        return !currButtons[ idx ][ button ] && prevButtons[ idx ][ button ];
    }

    // Returns true if a specific button on a specific controller is pressed.
    bool isButtonDown( PlayerIndex idx, SDL_GameControllerButton button ) const
    {
        return currButtons[ idx ][ button ];
    }

    // Returns the axis position of the left joystick on a specific controller.
    glm::vec2 leftStick( PlayerIndex idx ) const
    {
        return leftAxis[ idx ];
    }

    // Returns the axis position of the right joystick on a specific controller.
    glm::vec2 rightStick( PlayerIndex idx ) const
    {
        return rightAxis[ idx ];
    }

    // Returns the left trigger depression of a specific controller.
    float leftTrigger( PlayerIndex idx ) const
    {
        return triggerAxis[ idx ].x;
    }

    // Returns the right trigger depression of a specific controller.
    float rightTrigger( PlayerIndex idx ) const
    {
        return triggerAxis[ idx ].y;
    }

    // For a specific controller:
    // If threshold is positive, returns true if the left trigger
    // just crossed threshold in the pressed direction.
    // If threshold is negative, returns true if the left trigger
    // just crossed abs( threshold ) in the released direction.
    // Otherwise returns false.
    bool didLeftTriggerCross( PlayerIndex idx, float threshold ) const
    {
        float thr = std::abs( threshold );

        if ( threshold > 0 )
            return (triggerAxis[ idx ].x >= thr && prevTriggerAxis[ idx ].x < thr);

        return (triggerAxis[ idx ].x <= thr && prevTriggerAxis[ idx ].x > thr);
    }

    // For a specific controller:
    // If threshold is positive, returns true if the right trigger
    // just crossed threshold in the pressed direction.
    // If threshold is negative, returns true if the right trigger
    // just crossed abs( threshold ) in the released direction.
    // Otherwise returns false.
    bool didRightTriggerCross( PlayerIndex idx, float threshold ) const
    {
        float thr = std::abs( threshold );

        if ( threshold > 0 )
            return (triggerAxis[ idx ].y >= thr && prevTriggerAxis[ idx ].y < thr);

        return (triggerAxis[ idx ].y <= thr && prevTriggerAxis[ idx ].y > thr);
    }

};