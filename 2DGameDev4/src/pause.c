#include "pause.h"
#include "simple_logger.h"
#include "main_menu.h"
#include "buttons.h"
#include "sliders.h"
#include "options.h"

Bool paused = false;
Window *pause_window;
Widget pause_widget;
Widget pause_widget_exit;
Button pause_button;
Button pause_button_exit;
Widget pause_widget_restart;
Button pause_button_restart;
Widget pause_widget_mainmenu;
Button button_return_to_menu;
Widget pause_widget_options;
Button pause_button_options;

void pause_window_init()
{
	pause_window = window_new();
	if (!pause_window)
		return NULL;
	pause_window->sprite = gf2d_sprite_load_all("images/tileset.png", 32, 32, 4);
	pause_window->length = 500;
	pause_window->height = 500;
	pause_window->position = vector2d(350,100);
	pause_window->label = "Testing.";
	pause_window->font = TTF_OpenFont("fonts/BradleyGratis.ttf", 32);
	if (pause_window->font == NULL)
		slog("Unable to open a font for your window: %s", TTF_GetError());
	pause_window->update = window_update_generic;

	pause_window->parent = get_current_window();

	pause_window->widgets = pqlist_new();
	if (!pause_window->widgets) return NULL;

	pqlist_insert(pause_window->widgets, &pause_widget_options, 1);
	pause_widget_options.type = BUTTON_T;
	pause_widget_options.dimensions = vector2d(200, 100);
	pause_widget_options.position = vector2d(0, 400);
	pause_widget_options.widget.slider = &pause_button_options;
	pause_window->widgetCount += 1;

	pqlist_insert(pause_window->widgets, &pause_widget_exit, 1);
	pause_widget_exit.type = BUTTON_T;
	pause_widget_exit.dimensions = vector2d(200,100);
	pause_widget_exit.position = vector2d(0, 300);
	pause_widget_exit.widget.button = &pause_button_exit;
	pause_window->widgetCount += 1;

	pqlist_insert(pause_window->widgets, &pause_widget_mainmenu, 1);
	pause_widget_mainmenu.type = BUTTON_T;
	pause_widget_mainmenu.dimensions = vector2d(200, 100);
	pause_widget_mainmenu.position = vector2d(0, 200);
	pause_widget_mainmenu.widget.button = &button_return_to_menu;
	pause_window->widgetCount += 1;

	pqlist_insert(pause_window->widgets, &pause_widget_restart, 1);
	pause_widget_restart.type = BUTTON_T;
	pause_widget_restart.dimensions = vector2d(200, 100);
	pause_widget_restart.position = vector2d(0, 100);
	pause_widget_restart.widget.button = &pause_button_restart;
	pause_window->widgetCount += 1;

	pqlist_insert(pause_window->widgets, &pause_widget, 1);
	pause_widget.type = BUTTON_T;
	pause_widget.dimensions = vector2d(200,100);
	pause_widget.position = vector2d(0,0);
	pause_widget.widget.button = &pause_button;
	pause_window->widgetCount += 1;

	pause_button.onRelease = button_toggle_pause;
	pause_button.label = "Resume Game";

	pause_button_exit.label = "Close Game";
	pause_button_exit.onRelease = button_exit;

	pause_button_restart.label = "Restart";
	pause_button_restart.onRelease = button_restart_level;

	button_return_to_menu.label = "Return to Main Menu";
	button_return_to_menu.onRelease = button_return_to_main_menu;

	pause_button_options.label = "Options";
	pause_button_options.onRelease = options_window_init;
}

void pause_window_delete()
{
	window_close(pause_window);
}

void pause_toggle()
{
	paused = !paused;
	if (paused == false)
	{
		// delete the window
		pause_window_delete();
	}
	else
	{
		// create the window
		pause_window_init();
		set_current_window(pause_window);
	}
}

Bool paused_get()
{
	return paused;
}