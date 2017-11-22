#ifndef _INPUT_H_
#define _INPUT_H_

#include "gf2d_types.h"
#include <SDL.h>

typedef struct PlayerInput_s
{
	Bool charge;
	Bool select;
	Bool pull;
	Bool melee;

	double move_x;
	double move_y;
	
	double fwd_x;
	double fwd_y;
} PlayerInput;

typedef enum Input_Button_s
{
	INPUT_BUTTON_SELECT,
	INPUT_BUTTON_MELEE,
	INPUT_BUTTON_CHARGE,
	INPUT_BUTTON_PULL
} Input_Button;

typedef enum Input_Axis_s
{
	INPUT_AXIS_MOVE_X,
	INPUT_AXIS_MOVE_Y,
	INPUT_AXIS_FWD_X,
	INPUT_AXIS_FWD_Y
} Input_Axis;

void input_init();

void input_update_frame();

Bool input_get_button(int button);

int input_get_axis(int axis);

PlayerInput *input_get();

#endif