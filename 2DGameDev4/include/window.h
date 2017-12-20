#ifndef _WINDOW_H_
#define _WINDOW_H_

#include "gf2d_vector.h"
#include "gf2d_sprite.h"
#include "priority_queue.h"
#include <SDL_ttf.h>
#include "priority_queue.h"

typedef struct Button_s
{
	bool pressed;
	void(*onRelease)(struct Button_s *self);

	char *label;
	char *levelfile;

	Sprite *sprite;
	int buttonDown;
	int buttonUp;
} Button;

typedef struct Scroll_s
{
	bool pressed;
	void(*onRelease)(struct Scroll_s *self);

	PriorityQueueList *optionsList;
	int currentOption;

	Sprite *sprite;
	int leftArrow;
	int rightArrow;

	int timer;
	int lastMove;
	int moveDelay;
} Scroll;

typedef struct Slider_s
{
	int min;
	int max;
	int value;
	void(*set)(int value);

	char *label;

	Sprite *sprite;
	int left, middle, right;
	int mover;

	int lastMove;
	int moveDelay;
	int timer;
} Slider;

typedef enum
{
	BUTTON_T,
	SCROLL_T,
	SLIDER_T
} WIDGET_T;

typedef struct Widget_s
{
	WIDGET_T type;
	Vector2D dimensions;
	Vector2D position;
	union {
		Button *button;
		Slider *slider;
		Scroll *scroll;
	} widget;
} Widget;

typedef struct Window_s
{
	Vector2D position;
	bool inUse;
	float length, height;

	TTF_Font *font;
	char *label;

	Sprite *sprite;
	int tl, tm, tr;
	int ml, mm, mr;
	int bl, bm, br;
	
	void(*update)(struct Window_s *self);

	PriorityQueueList *widgets;
	int selectedWidget;
	int widgetCount;

	int lastInput;

	struct Window_s *parent;
} Window;

void window_system_init(Uint32 max);

Window *get_current_window();

void set_current_window(Window *win);

Window *window_new();

void window_free(Window *win);

void window_close(Window *win);

void window_update_generic(Window *self);

void window_update_all();

void window_draw_all();

void window_system_close();

void window_clear_all();

#endif