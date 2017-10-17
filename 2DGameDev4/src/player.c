#include <SDL.h>
#include "player.h"
#include "simple_logger.h"
#include "testUpdate.h"
#include "gf2d_draw.h"
#include "config_loader.h"

SDL_GameController *controller;
SDL_Joystick *joystick;

Vector2D direction = { 200,0 };

Entity *player;

void set_player(Entity *ent)
{
	player = ent;
}
Entity *get_player()
{
	return player;
}

void player_init(Entity *self)
{
	int i;
	controller = NULL;
	joystick = NULL;
	set_player(self);
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
	if (self->coll != NULL)
	{
		self->eyePos = vector2d(
			self->coll->width / 2,
			self->coll->height / 2);
	}
}

void player_update(Entity *self)
{
	Entity *projectile;
	int mx, my;
	Vector2D direction;
	Vector2D eyePos;

	vector2d_add(eyePos, self->position, self->eyePos);

	direction = self->forward;
	direction = vector2d_rotate(direction, 90 * GF2D_PI / -360);
	self->lastPosition = self->position;
	if (!controller)
	{
		SDL_GetMouseState(&mx, &my);
		self->position = vector2d(mx, my);
	}
	else
	{
		//self->velocity.x = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX) / (double)10000;
		//self->velocity.y = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY) / (double)10000;
		self->forward = vector2d(SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTX), SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTY));
		vector2d_set_magnitude(&self->forward, 300);
		if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_A))
		{
			if (self->projectile != NULL)
			{
				projectile = entity_new();
				if (projectile != NULL)
				{
					copy_prefab(projectile, self->projectile);
					projectile->position = self->position;
					projectile->velocity = self->velocity;
				}
			}
			else
			{
				slog("no projectile to spawn");
			}
		}
	}
	draw_line_of_sight(self, 1, 90, direction, vector4d(255,255,70,255), 10, eyePos);
	self->timer += 1;
}

void player_touch(Entity *self, Entity *other)
{
	if (self->timer - self->lastHit > 60)
	{
		self->colorShift.x -= 5;
		self->lastHit = self->timer;
		sound_play(self->ouch, 0, self->ouch->volume, self->ouch->defaultChannel, 0);
	}
	if (other->layer == 1)
	{
		self->position = self->lastPosition;
	}
}