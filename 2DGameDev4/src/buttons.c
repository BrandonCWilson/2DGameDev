#include "buttons.h"
#include "gamestate.h"
#include "pause.h"
#include "simple_logger.h"

void button_return_to_main_menu(Button *self)
{
	game_close_map();
	pause_toggle();
	main_menu_toggle();
}

void button_exit(Button *self)
{
	game_set_done_true();
}

void button_start_level(Button *self)
{
	if (!self) return;
	slog("loading level: %s", self->levelfile);
	game_load_map(self->levelfile);
	main_menu_toggle();
}

void button_toggle_pause(Button *self)
{
	pause_toggle();
}

void button_restart_level(Button *self)
{
	game_restart_map(get_current_tilemap(), "levels/tilemap.map");
	pause_toggle();
}