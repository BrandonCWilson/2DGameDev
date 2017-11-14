#ifndef _WINDOW_H_
#define _WINDOW_H_

#include "gf2d_vector.h"
#include "gf2d_sprite.h"

typedef struct Window_s
{
	Vector2D position;
	bool inUse;
	float length, height;
	Sprite *sprite;
	int tl, tm, tr;
	int ml, mm, mr;
	int bl, bm, br;
	void(*update)(struct Window_s *self);
} Window;

void window_system_init(Uint32 max);

Window *window_new();

void window_delete(Window *win);

void window_free(Window *win);

void window_update_all();

void window_draw_all();

void window_system_close();

void window_clear_all();

#endif