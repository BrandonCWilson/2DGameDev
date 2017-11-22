#include "gamestate.h"

Bool done = false;

Bool game_done()
{
	return done;
}

void game_set_done_true()
{
	done = true;
}