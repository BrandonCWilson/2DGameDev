#include "main_menu.h"
#include "simple_logger.h"
#include "input.h"
#include "buttons.h"

Bool is_open = false;

Window *win = NULL;
Widget menu_start_level;
Button button_level1;
Widget menu_exit;
Button button_exit_game;

void main_menu_init()
{
	Window *menu = NULL;
	menu = window_new();
	if (!menu) return NULL;
	win = menu;

	menu->sprite = gf2d_sprite_load_all("images/tileset.png", 32, 32, 16);
	menu->length = 500;
	menu->height = 500;
	menu->position = vector2d(350, 100);
	menu->label = "Testing.";
	menu->font = TTF_OpenFont("fonts/BradleyGratis.ttf", 32);
	if (menu->font == NULL)
		slog("Unable to open a font for your window: %s", TTF_GetError());
	menu->update = window_update_generic;

	menu->widgets = pqlist_new();
	if (!menu->widgets) return NULL;

	pqlist_insert(menu->widgets, &menu_exit, 1);
	menu_exit.type = BUTTON_T;
	menu_exit.widget.button = &button_exit_game;
	menu_exit.dimensions = vector2d(200, 75);
	menu_exit.position = vector2d(0, 150);
	menu->widgetCount += 1;

	pqlist_insert(menu->widgets, &menu_start_level, 1);
	menu_start_level.type = BUTTON_T;
	menu_start_level.widget.button = &button_level1;
	menu_start_level.dimensions = vector2d(200, 75);
	menu_start_level.position = vector2d(0, 0);
	menu->widgetCount += 1;

	button_level1.label = "Start level 1";
	button_level1.levelfile = "levels/tilemap.map";
	button_level1.onRelease = button_start_level;

	button_exit_game.label = "Exit Game";
	button_exit_game.onRelease = button_exit;

	is_open = true;
}

void main_menu_close()
{
	window_free(win);
	is_open = false;
}

void main_menu_toggle()
{
	if (!is_open)
	{
		main_menu_init();
		set_current_window(win);
	}
	else
	{
		main_menu_close();
	}
}