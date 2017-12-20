#include "main_menu.h"
#include "simple_logger.h"
#include "input.h"
#include "buttons.h"
#include <physfs.h>
#include "config_loader.h"
#include "scrolls.h"

Bool is_open = false;

Window *win = NULL;

Widget menu_level_select;
Scroll scroll_level_select;

Widget menu_start_level;
Button button_level1;

Widget menu_exit;
Button button_exit_game;

Widget menu_start_map_editor;
Button button_map_editor;

void main_menu_init()
{
	Window *menu = NULL;
	SDL_RWops *rw;
	menu = window_new();
	if (!menu) return NULL;
	win = menu;

	menu->sprite = gf2d_sprite_load_all("images/tileset.png", 32, 32, 4);
	menu->length = 500;
	menu->height = 500;
	menu->position = vector2d(350, 100);
	menu->label = "Testing.";
	if ((rw = PHYSFSRWOPS_openRead("fonts/BradleyGratis.ttf")) == NULL)
	{
		slog("Could not load font for pause window.");
		return NULL;
	}
	menu->font = TTF_OpenFontRW(rw, 1, 32);
	if (menu->font == NULL)
		slog("Unable to open a font for your window: %s", TTF_GetError());
	menu->update = window_update_generic;

	menu->widgets = pqlist_new();
	if (!menu->widgets) return NULL;

	pqlist_insert(menu->widgets, &menu_start_map_editor, 1);
	menu_start_map_editor.type = BUTTON_T;
	menu_start_map_editor.widget.button = &button_map_editor;
	menu_start_map_editor.dimensions = vector2d(200, 75);
	menu_start_map_editor.position = vector2d(0, 225);
	menu->widgetCount += 1;

	pqlist_insert(menu->widgets, &menu_exit, 1);
	menu_exit.type = BUTTON_T;
	menu_exit.widget.button = &button_exit_game;
	menu_exit.dimensions = vector2d(200, 75);
	menu_exit.position = vector2d(0, 150);
	menu->widgetCount += 1;

	pqlist_insert(menu->widgets, &menu_level_select, 1);
	menu_level_select.type = SCROLL_T;
	menu_level_select.widget.button = &scroll_level_select;
	menu_level_select.dimensions = vector2d(200, 75);
	menu_level_select.position = vector2d(0, 75);
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

	button_map_editor.label = "Map Editor";
	button_map_editor.onRelease = button_start_map_editor;

	scroll_level_select.optionsList = level_list_get();
	scroll_level_select.onRelease = scroll_start_level;
	scroll_level_select.moveDelay = 20;

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