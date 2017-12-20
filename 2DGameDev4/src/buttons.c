#include "buttons.h"
#include "gamestate.h"
#include "pause.h"
#include "simple_logger.h"
#include "map_editor.h"
#include "main_menu.h"

void button_return_to_main_menu(Button *self)
{
	if (game_is_editing())
	{
		map_editor_close();
	}
	game_close_map();
	pause_toggle();
	main_menu_toggle();
}

void button_start_map_editor(Button *self)
{
	slog("I'd like to start the map editor!");
	game_start_map_editor();
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
	game_restart_map(get_current_tilemap(), get_current_tilemap()->filepath);
	pause_toggle();
}