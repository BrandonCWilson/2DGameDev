#include "window.h"
#include "simple_logger.h"
#include "gf2d_graphics.h"
#include <stdio.h>
#include "input.h"
#include <SDL2_gfxPrimitives.h>

typedef struct
{
	Uint32 max_windows;
	Window *win_list;
} WinManager;

static WinManager window_manager;

Window *current_window;

void set_current_window(Window *win)
{
	current_window = win;
}

Window *get_current_window()
{
	return current_window;
}

void widget_update_button(Button *button)
{
	if (input_get_button(INPUT_BUTTON_SELECT))
	{
		button->pressed = true;
		button->label = "SELECTED";
	}
	else
	{
		if (button->pressed == true)
		{
			slog("Button released.");
			if (button->onRelease != NULL)
				button->onRelease(button);
			else
				slog("No onrelease function");
			slog("Onrelease");
		}
		button->pressed = false;
	}
}

void widget_update_slider(Slider *slider)
{

}

void widget_update_generic(Widget *widget)
{
	if (!widget) return;
	if (widget->type == BUTTON_T) widget_update_button(widget->widget.button);
	if (widget->type == SLIDER_T) widget_update_slider(widget->widget.slider);
}

void window_update_generic(Window *self)
{
	Widget *selected;
	PriorityNode *cursor;
	int i;
	if (!self) return;
	if (get_current_window() != self) return;
	if (!self->widgets) return;
	if (!self->widgets->head) return;
	cursor = self->widgets->head;
	if (input_get_axis(INPUT_AXIS_MOVE_Y) > 20000)
	{
		if (self->lastInput <= 0)
		{
			self->selectedWidget += 1;
			if (self->selectedWidget >= self->widgetCount)
				self->selectedWidget = self->widgetCount - 1;
		}
		self->lastInput = 1;
	}
	else if (input_get_axis(INPUT_AXIS_MOVE_Y) < -20000)
	{
		if (self->lastInput >= 0)
		{
			self->selectedWidget -= 1;
			if (self->selectedWidget < 0)
				self->selectedWidget = 0;
		}
		self->lastInput = -1;
	}
	else
		self->lastInput = 0;
	for (i = 0; i < self->widgetCount; i++)
	{
		if (!cursor) return;
		if (i == self->selectedWidget) break;
		cursor = cursor->next;
	}
	if (!cursor->data) return;
	// okay, we should probably have a pointer to a widget at this point
	// since that widget is selected, we should probably be concerned with its update
	//slog("Updating widget: %i", i);
	widget_update_generic(cursor->data);
}

void window_system_init(Uint32 max)
{
	int i;
	slog("initializing window system..");
	if (!max)
	{
		slog("cannot initialize a window system for zero windows!");
		return;
	}
	window_manager.max_windows = max;
	window_manager.win_list = (Window *)malloc(sizeof(Window)*max);
	memset(window_manager.win_list, 0, sizeof(Window)*max);

	slog("window system initialized");
	atexit(window_system_close);
}

void test_widgets()
{
	Window *win;
	Widget *widget;
}

void window_update_all()
{
	int i;
	for (i = 0; i < window_manager.max_windows; i++)
	{
		if (window_manager.win_list[i].inUse)
		{
			if (window_manager.win_list[i].update != NULL)
				window_manager.win_list[i].update(&window_manager.win_list[i]);
		}
	}
}

void widget_draw_button(Button *button, Vector2D position, Vector2D dimensions, TTF_Font *font)
{
	SDL_Color White = { 255, 255, 255 };
	SDL_Surface *surfaceMessage;
	SDL_Texture *message;
	SDL_Rect Message_rect;
	SDL_Surface *renderer;
	if (!button) return;
	//if (!button->sprite) return;
	if (font != NULL)
	{
		renderer = gf2d_graphics_get_renderer();
		surfaceMessage = TTF_RenderText_Solid(font, button->label, White);
		message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);
		Message_rect.x = position.x; 
		Message_rect.y = position.y; 
		Message_rect.w = dimensions.x; 
		Message_rect.h = dimensions.y; 

		SDL_RenderCopy(renderer, message, NULL, &Message_rect);
	}
	else
		slog("No font!");
}

void widget_draw_slider(Slider *slider, Vector2D position, TTF_Font *font)
{
	if (!slider) return;
	if (!slider->sprite) return;
}

void widget_draw_generic(Widget *widget, Vector2D position, TTF_Font *font)
{
	if (!widget) return;
	if (widget->type == BUTTON_T)
		widget_draw_button(widget->widget.button, position, widget->dimensions, font);
	if (widget->type == SLIDER_T)
		widget_draw_slider(widget->widget.slider, position, font);
}

void window_draw_widgets(Window *window)
{
	int i;
	PriorityNode *cursor;
	Vector2D pos;
	if (!window) return;
	if (!window->widgets) return;
	if (!window->widgets->head) return;
	cursor = window->widgets->head;
	for (i = 0; i < window->widgetCount; i++)
	{
		if (!cursor) break;
		if (!cursor->data) break;
		//FIXME
		// dynamically figure out where in the window to draw widgets in sequence
		// maybe use a variable in the config??
		// only using window->position as a temp variable that I know will be 
		vector2d_add(pos, window->position, ((Widget *)cursor->data)->position);
		vector2d_add(pos, pos, vector2d(window->sprite->frame_w, window->sprite->frame_h));
		if (i == window->selectedWidget)
		{
			circleRGBA(gf2d_graphics_get_renderer(), pos.x, pos.y, 25, 255, 255, 255, 255);
		}
		widget_draw_generic(cursor->data, pos, window->font);
		cursor = cursor->next;
	}
}

void window_draw_all()
{
	int i;
	Vector4D colorshift;
	Vector2D scale;
	for (i = 0; i < window_manager.max_windows; i++)
	{
		if (window_manager.win_list[i].inUse)
		{
			if (window_manager.win_list[i].sprite != NULL)
			{
				scale = vector2d((window_manager.win_list[i].length - 2 * window_manager.win_list[i].sprite->frame_w) / window_manager.win_list[i].sprite->frame_w, (window_manager.win_list[i].height - 2 * window_manager.win_list[i].sprite->frame_h) / window_manager.win_list[i].sprite->frame_h);
				gf2d_sprite_draw(window_manager.win_list[i].sprite,
					vector2d(window_manager.win_list[i].position.x + window_manager.win_list[i].sprite->frame_w, window_manager.win_list[i].position.y + window_manager.win_list[i].sprite->frame_h),
					&scale,
					NULL,
					NULL,
					NULL,
					NULL,
					window_manager.win_list[i].mm);
				// widget drawing, labels, go here, behind the borders
				window_draw_widgets(&window_manager.win_list[i]);

				// draw the 8 border sprites as necessary based on window dimensions, scale them accordingly
				colorshift = vector4d(255, 0, 0, 255);
				gf2d_sprite_draw(window_manager.win_list[i].sprite,
					// position vector
					window_manager.win_list[i].position,
					NULL,
					NULL,
					NULL,
					NULL,
					&colorshift,
					window_manager.win_list[i].tl);
				colorshift = vector4d(0, 255, 0, 255);
				scale = vector2d((window_manager.win_list[i].length - 2 * window_manager.win_list[i].sprite->frame_w) / window_manager.win_list[i].sprite->frame_w, 1);
				gf2d_sprite_draw(window_manager.win_list[i].sprite,
					// position vector
					vector2d(window_manager.win_list[i].position.x + window_manager.win_list[i].sprite->frame_w, window_manager.win_list[i].position.y),
					// FIXME
					// scale accordingly
					&scale,
					NULL,
					NULL,
					NULL,
					&colorshift,
					window_manager.win_list[i].tm);
				colorshift = vector4d(0, 0, 255, 255);
				gf2d_sprite_draw(window_manager.win_list[i].sprite,
					// position vector
					vector2d(window_manager.win_list[i].position.x + window_manager.win_list[i].length - window_manager.win_list[i].sprite->frame_w, window_manager.win_list[i].position.y),
					NULL,
					NULL,
					NULL,
					NULL,
					&colorshift,
					window_manager.win_list[i].tr);


				scale = vector2d(1, (window_manager.win_list[i].height - 2 * window_manager.win_list[i].sprite->frame_h) / window_manager.win_list[i].sprite->frame_h);
				gf2d_sprite_draw(window_manager.win_list[i].sprite,
					vector2d(window_manager.win_list[i].position.x, window_manager.win_list[i].position.y + window_manager.win_list[i].sprite->frame_h),
					&scale,
					NULL,
					NULL,
					NULL,
					NULL,
					window_manager.win_list[i].ml);
				scale = vector2d(1, (window_manager.win_list[i].height - 2 * window_manager.win_list[i].sprite->frame_h) / window_manager.win_list[i].sprite->frame_h);
				gf2d_sprite_draw(window_manager.win_list[i].sprite,
					vector2d(window_manager.win_list[i].position.x + window_manager.win_list[i].length - window_manager.win_list[i].sprite->frame_w, window_manager.win_list[i].position.y + window_manager.win_list[i].sprite->frame_h),
					&scale,
					NULL,
					NULL,
					NULL,
					NULL,
					window_manager.win_list[i].mr);

				colorshift = vector4d(255, 0, 0, 255);
				gf2d_sprite_draw(window_manager.win_list[i].sprite,
					// position vector
					vector2d(window_manager.win_list[i].position.x, window_manager.win_list[i].position.y + window_manager.win_list[i].height - window_manager.win_list[i].sprite->frame_h),
					NULL,
					NULL,
					NULL,
					NULL,
					&colorshift,
					window_manager.win_list[i].bl);
				colorshift = vector4d(0, 255, 0, 255);
				scale = vector2d((window_manager.win_list[i].length - 2 * window_manager.win_list[i].sprite->frame_w) / window_manager.win_list[i].sprite->frame_w, 1);
				gf2d_sprite_draw(window_manager.win_list[i].sprite,
					// position vector
					vector2d(window_manager.win_list[i].position.x + window_manager.win_list[i].sprite->frame_w, window_manager.win_list[i].position.y + window_manager.win_list[i].height - window_manager.win_list[i].sprite->frame_h),
					// FIXME
					// scale accordingly
					&scale,
					NULL,
					NULL,
					NULL,
					&colorshift,
					window_manager.win_list[i].bm);
				colorshift = vector4d(0, 0, 255, 255);
				gf2d_sprite_draw(window_manager.win_list[i].sprite,
					// position vector
					vector2d(window_manager.win_list[i].position.x + window_manager.win_list[i].length - window_manager.win_list[i].sprite->frame_w, window_manager.win_list[i].position.y + window_manager.win_list[i].height - window_manager.win_list[i].sprite->frame_h),
					NULL,
					NULL,
					NULL,
					NULL,
					&colorshift,
					window_manager.win_list[i].br);
			}
		}
	}
}

Window *window_new()
{
	int i;
	/*search for an unused entity address*/
	for (i = 0; i < window_manager.max_windows; i++)
	{
		if (!window_manager.win_list[i].inUse)
		{
			entity_delete(&window_manager.win_list[i]);	// clean up the old data
			window_manager.win_list[i].inUse = true;	// set it to inUse
			return &window_manager.win_list[i];			// return address of this array element
		}
	}
	slog("error: out of entity addresses");
	return NULL;
}

void window_free(Window *win)
{
	if (!win)
		return;
	win->inUse = false;
}

void window_close(Window *win)
{
	if (!win)
		return;
	if (get_current_window() == win)
		set_current_window(win->parent);
	window_free(win);
}

void window_delete(Window *win)
{
	if (!win)
		return;
	if (win->sprite != NULL)
		gf2d_sprite_free(win->sprite);
	if (win->widgets != NULL)
	{
		pqlist_free(win->widgets, NULL);
	}
	memset(win, 0, sizeof(Window));
}

void window_clear_all()
{
	int i;
	for (i = 0; i < window_manager.max_windows; i++)
	{
		window_delete(&window_manager.win_list[i]);
	}
}

void window_system_close()
{
	window_clear_all();
	if (window_manager.win_list != NULL)
	{
		free(window_manager.win_list);
	}
	window_manager.win_list = NULL;
	window_manager.max_windows = 0;
}