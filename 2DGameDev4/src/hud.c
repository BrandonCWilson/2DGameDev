#include "hud.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include "simple_logger.h"
#include "map.h"

int baseTime;
int timer;
TTF_Font *font;

void hud_init()
{
	SDL_RWops *rw;
	baseTime = SDL_GetTicks();
	timer = baseTime;
	if ((rw = PHYSFSRWOPS_openRead("fonts/BradleyGratis.ttf")) == NULL)
	{
		slog("Could not load font for pause window.");
		return NULL;
	}
	font = TTF_OpenFontRW(rw, 1, 32);
	if (font == NULL)
		slog("Unable to open a font for your window: %s", TTF_GetError());
}

void hud_update()
{
	timer = SDL_GetTicks();
}

void hud_draw()
{
	char clock[16];
	int time;
	float seconds;
	char remaining[128];

	SDL_Color White = { 255, 255, 255 };
	SDL_Surface *surfaceMessage;
	SDL_Texture *message;
	SDL_Rect Message_rect;
	SDL_Surface *renderer;

	time = timer - baseTime;
	seconds = time / 1000.0;
	sprintf(clock, "%f", seconds);

	renderer = gf2d_graphics_get_renderer();
	surfaceMessage = TTF_RenderText_Solid(font, clock, White);
	message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);
	Message_rect.x = 400;
	Message_rect.y = 0;
	Message_rect.w = 300;
	Message_rect.h = 150;

	SDL_RenderCopy(renderer, message, NULL, &Message_rect);

	sprintf(remaining, "%i left", get_current_tilemap()->numEnemies - 1);
	
	surfaceMessage = TTF_RenderText_Solid(font, remaining, White);
	Message_rect.x = 500;
	Message_rect.y += 150;
	Message_rect.w = 100;
	Message_rect.h = 50;
	message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);

	SDL_RenderCopy(renderer, message, NULL, &Message_rect);
}