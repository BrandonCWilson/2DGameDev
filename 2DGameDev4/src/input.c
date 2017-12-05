#include "input.h"
#include "simple_logger.h"

PlayerInput player_input;

SDL_GameController *controller = NULL;
SDL_Joystick *joystick;

// FIXME
// support non-axis controller bindings for charge and pull
int CONTROLLER_SELECT = SDL_CONTROLLER_BUTTON_A;
int CONTROLLER_MELEE = SDL_CONTROLLER_BUTTON_X;
int CONTROLLER_HURT_SELF = SDL_CONTROLLER_BUTTON_B;
int CONTROLLER_CHARGE = SDL_CONTROLLER_AXIS_TRIGGERRIGHT;
int CONTROLLER_PULL = SDL_CONTROLLER_AXIS_TRIGGERLEFT;

// FIXME
// support d-pad controller bindings for move and fwd
int CONTROLLER_MOVE_X = SDL_CONTROLLER_AXIS_LEFTX;
int CONTROLLER_MOVE_Y = SDL_CONTROLLER_AXIS_LEFTY;

int CONTROLLER_FWD_X = SDL_CONTROLLER_AXIS_RIGHTX;
int CONTROLLER_FWD_Y = SDL_CONTROLLER_AXIS_RIGHTY;

void input_init()
{
	int i;
	controller = NULL;
	joystick = NULL;
	for (i = 0; i < SDL_NumJoysticks(); ++i) {
		if (SDL_IsGameController(i)) {
			slog("Index \'%i\' is a compatible controller, named \'%s\'\n", i, SDL_GameControllerNameForIndex(i));
			controller = SDL_GameControllerOpen(i);
		}
		else {
			slog("Index \'%i\' is not a compatible controller.\n", i);
		}
	}
	if (!controller)
		slog("Unable to find a controller..");
}

void input_clear_frame()
{
	memset(&player_input, 0, sizeof(PlayerInput));
}

void input_update_frame()
{
	input_clear_frame();

	// FIXME
	// check if any controller has been removed, or added

	// controller controls
	if (controller != NULL)
	{
		if (SDL_GameControllerGetButton(controller, CONTROLLER_SELECT))
		{
			player_input.select = true;
		}
		if (SDL_GameControllerGetButton(controller, CONTROLLER_MELEE))
		{
			player_input.melee = true;
		}
		if (SDL_GameControllerGetButton(controller, CONTROLLER_HURT_SELF))
		{
			player_input.hurt_self = true;
		}
		// FIXME
		// support non-axis controller bindings for charge and pull
		if (SDL_GameControllerGetAxis(controller, CONTROLLER_PULL) > 1000)
		{
			player_input.pull = true;
		}
		if (SDL_GameControllerGetAxis(controller, CONTROLLER_CHARGE) > 1000)
		{
			player_input.charge = true;
		}
		// FIXME
		// support d-pad controller bindings for movement and fwd
		player_input.move_x = SDL_GameControllerGetAxis(controller, CONTROLLER_MOVE_X);
		player_input.move_y = SDL_GameControllerGetAxis(controller, CONTROLLER_MOVE_Y);
		player_input.fwd_x = SDL_GameControllerGetAxis(controller, CONTROLLER_FWD_X);
		player_input.fwd_y = SDL_GameControllerGetAxis(controller, CONTROLLER_FWD_Y);
	}
	// keyboard&mouse controls
	else
	{
		//slog("You have no controller.");
	}
}

Bool input_get_button(int button)
{
	if (button == INPUT_BUTTON_SELECT)
	{
		return player_input.select;
	}
	else if (button == INPUT_BUTTON_CHARGE)
	{
		return player_input.charge;
	}
	else if (button == INPUT_BUTTON_MELEE)
	{
		return player_input.melee;
	}
	else if (button == INPUT_BUTTON_PULL)
	{
		return player_input.pull;
	}
	else if (button == INPUT_HURT_SELF)
	{
		return player_input.hurt_self;
	}
	else
	{
		return false;
	}
}

int input_get_axis(int axis)
{
	if (axis == INPUT_AXIS_FWD_X)
	{
		return player_input.fwd_x;
	}
	else if (axis == INPUT_AXIS_FWD_Y)
	{
		return player_input.fwd_y;
	}
	else if (axis == INPUT_AXIS_MOVE_X)
	{
		return player_input.move_x;
	}
	else if (axis == INPUT_AXIS_MOVE_Y)
	{
		return player_input.move_y;
	}
	else
	{
		return NULL;
	}
}

Bool input_is_controller()
{
	if (!controller) return false;
	return true;
}