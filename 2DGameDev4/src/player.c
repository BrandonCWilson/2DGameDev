#include <SDL.h>
#include "player.h"
#include "simple_logger.h"
#include "testUpdate.h"

SDL_GameController *controller;
SDL_Joystick *joystick;

int i = 0;
Vector2D direction = { 200,0 };

void player_init(Entity *self)
{
	int i;
	controller = NULL;
	joystick = NULL;
	for (i = 0; i < SDL_NumJoysticks(); ++i) {
		if (SDL_IsGameController(i)) {
			slog("Index \'%i\' is a compatible controller, named \'%s\'\n", i, SDL_GameControllerNameForIndex(i));
			controller = SDL_GameControllerOpen(i);
			joystick = SDL_GameControllerGetJoystick(controller);
		}
		else {
			slog("Index \'%i\' is not a compatible controller.\n", i);
		}
	}
	if (!controller)
		slog("Unable to find a controller..");
}

void player_update(Entity *self)
{
	Entity *clickEnt;
	int mx, my;
	if (!controller)
	{
		SDL_GetMouseState(&mx, &my);
		//self->position = vector2d(mx, my);
	}
	else
	{
		self->position.x += SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX) / (double)10000;
		self->position.y += SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY) / (double)10000;

		if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_A))
		{
			clickEnt = entity_new();
			if (clickEnt != NULL)
			{
				clickEnt->sprite = self->sprite;
				clickEnt->timer = 0;
				clickEnt->position = vector2d(self->position.x, self->position.y);
				clickEnt->velocity = vector2d(10.0, 0);
				clickEnt->update = clickerUpdate;
				clickEnt->colorShift = vector4d((int)(rand() % 255), (int)(rand() % 255), (int)(rand() % 255), 255);
			}
		}
	}
	draw_line_of_sight(self, 1, 90, direction, vector4d(0,0,100,0), 20);
	direction = vector2d_rotate(direction, 0.03);
	i++;
	self->timer += 1;
}

void player_touch(Entity *self, Entity *other)
{
	if (self->timer - self->lastHit > 60)
	{
		self->colorShift.x -= 5;
		self->lastHit = self->timer;
		sound_play(self->ouch, 0, 50, 1, 0);
	}
}