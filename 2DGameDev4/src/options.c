#include "options.h"
#include "sliders.h"
#include "simple_logger.h"
#include "audio.h"

Window *options_window;

Widget widget_volume_master;
Slider slider_volume_master;

Widget widget_volume_music;
Slider slider_volume_music;

Widget widget_volume_sound;
Slider slider_volume_sound;

Widget widget_close_options;
Button button_close_options;

void options_window_close()
{
	if (options_window != NULL)
		window_close(options_window);
}

void button_window_close(Button *self)
{
	options_window_close();
}

void options_window_init()
{
	options_window = window_new();
	if (!options_window) return NULL;

	options_window->sprite = gf2d_sprite_load_all("images/tileset.png", 32, 32, 4);
	options_window->length = 500;
	options_window->height = 500;
	options_window->position = vector2d(350, 100);
	options_window->label = "Testing.";
	options_window->font = TTF_OpenFont("fonts/BradleyGratis.ttf", 32);
	if (options_window->font == NULL)
		slog("Unable to open a font for your window: %s", TTF_GetError());
	options_window->update = window_update_generic;

	options_window->parent = get_current_window();

	options_window->widgets = pqlist_new();
	if (!options_window->widgets) return NULL;

	pqlist_insert(options_window->widgets, &widget_close_options, 1);
	widget_close_options.type = BUTTON_T;
	widget_close_options.dimensions = vector2d(200, 100);
	widget_close_options.position = vector2d(0, 300);
	widget_close_options.widget.button = &button_close_options;
	options_window->widgetCount += 1;

	pqlist_insert(options_window->widgets, &widget_volume_sound, 1);
	widget_volume_sound.type = SLIDER_T;
	widget_volume_sound.dimensions = vector2d(200, 100);
	widget_volume_sound.position = vector2d(0, 200);
	widget_volume_sound.widget.slider = &slider_volume_sound;
	options_window->widgetCount += 1;

	pqlist_insert(options_window->widgets, &widget_volume_music, 1);
	widget_volume_music.type = SLIDER_T;
	widget_volume_music.dimensions = vector2d(200, 100);
	widget_volume_music.position = vector2d(0, 100);
	widget_volume_music.widget.slider = &slider_volume_music;
	options_window->widgetCount += 1;

	pqlist_insert(options_window->widgets, &widget_volume_master, 1);
	widget_volume_master.type = SLIDER_T;
	widget_volume_master.dimensions = vector2d(200, 100);
	widget_volume_master.position = vector2d(0, 0);
	widget_volume_master.widget.slider = &slider_volume_master;
	options_window->widgetCount += 1;

	slider_volume_master.sprite = gf2d_sprite_load_all("images/tileset.png", 32, 32, 4);
	slider_volume_master.left = 0;
	slider_volume_master.middle = 1;
	slider_volume_master.right = 2;
	slider_volume_master.mover = 3;
	slider_volume_master.max = 10;
	slider_volume_master.set = slider_set_master_volume;
	slider_volume_master.value = audio_get_master_volume();

	slider_volume_music.sprite = gf2d_sprite_load_all("images/tileset.png", 32, 32, 4);
	slider_volume_music.left = 0;
	slider_volume_music.middle = 1;
	slider_volume_music.right = 2;
	slider_volume_music.mover = 3;
	slider_volume_music.max = 10;
	slider_volume_music.set = slider_set_music_volume;
	slider_volume_music.value = audio_get_music_volume();

	slider_volume_sound.sprite = gf2d_sprite_load_all("images/tileset.png", 32, 32, 4);
	slider_volume_sound.left = 0;
	slider_volume_sound.middle = 1;
	slider_volume_sound.right = 2;
	slider_volume_sound.mover = 3;
	slider_volume_sound.max = 10;
	slider_volume_sound.set = slider_set_sound_effect_volume;
	slider_volume_sound.value = audio_get_sound_volume();

	button_close_options.label = "Close Options";
	button_close_options.onRelease = button_window_close;

	set_current_window(options_window);
}