#include "pause.h"
#include "simple_logger.h"

Bool paused = false;
Window *pause_window;
Widget pause_widget;
Widget pause_widget_exit;
Button pause_button;
Button pause_button_exit;

void pause_button_exit_released(Button *self)
{
	game_set_done_true();
}

void pause_button_released(Button *self)
{
	pause_toggle();
}

void pause_window_init()
{
	pause_window = window_new();
	if (!pause_window)
		return NULL;
	pause_window->sprite = gf2d_sprite_load_all("images/tileset.png", 32, 32, 16);
	pause_window->length = 500;
	pause_window->height = 500;
	pause_window->position = vector2d(350,100);
	pause_window->label = "Testing.";
	pause_window->font = TTF_OpenFont("fonts/BradleyGratis.ttf", 32);
	if (pause_window->font == NULL)
		slog("Unable to open a font for your window: %s", TTF_GetError());
	pause_window->update = window_update_generic;

	pause_window->widgets = pqlist_new();

	pqlist_insert(pause_window->widgets, &pause_widget_exit, 1);
	pause_widget_exit.type = BUTTON_T;
	pause_widget_exit.dimensions = vector2d(150, 150);
	pause_widget_exit.position = vector2d(0, 300.5);
	pause_widget_exit.widget.button = &pause_button_exit;
	pause_window->widgetCount += 1;

	pqlist_insert(pause_window->widgets, &pause_widget, 1);
	pause_widget.type = BUTTON_T;
	pause_widget.dimensions = vector2d(300.5, 300.5);
	pause_widget.position = vector2d(0,0);
	pause_widget.widget.button = &pause_button;
	pause_window->widgetCount += 1;

	pause_button.onRelease = pause_button_released;
	pause_button.label = "Pause";

	pause_button_exit.label = "Exit";
	pause_button_exit.onRelease = pause_button_exit_released;
}

void pause_window_delete()
{
	window_free(pause_window);
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
	}
}

Bool paused_get()
{
	return paused;
}