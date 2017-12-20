#include <SDL.h>
#include "player.h"
#include "simple_logger.h"
#include "testUpdate.h"
#include "gf2d_draw.h"
#include "config_loader.h"
#include "input.h"
#include "sound_detection.h"

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

void player_draw(Entity *self)
{
	Vector2D eyePos;
	vector2d_add(eyePos, self->position, self->eyePos);
	direction = self->forward;
	direction = vector2d_rotate(direction, self->fov * GF2D_PI / -360);
	draw_line_of_sight(self, 1, self->fov, direction, vector4d(255, 255, 70, 255), 15, eyePos);
}

void player_init(Entity *self)
{
	int i;
	controller = NULL;
	joystick = NULL;
	set_player(self);
	self->draw = player_draw;
	vector2d_add(self->eyePos, self->position, vector2d(self->coll->width / 2, self->coll->height / 2));

	self->animator = animator_new("Player", self->sprite);
	animation_new("Player", "Idle", 1, 3, -1, "Idle", NULL, 8);
	strcpy(self->currentAnimation, "Idle");
}

void player_eat(Entity *self, Vector2D eyePos)
{
	Entity *nearestFood;
	Vector2D direction, diff;
	if (self->holding == NULL)
	{
		direction = vector2d_rotate(self->forward, self->fov * GF2D_PI / -360);
		nearestFood = entity_closest_in_sight_by_layer(self, 5, eyePos, direction);
		if (nearestFood != NULL)
		{
			vector2d_sub(diff, eyePos, nearestFood->position);
			if (vector2d_magnitude_squared(diff) < 6000)
			{
				nearestFood->die(nearestFood);
				self->killCount += 1;
				slog("I ate something meaty");
			}
		}
	}
}

void player_shoot(Entity *self)
{
	Entity *projectile;
	if (self->timer - self->lastShot < self->reload)
	{
		return;
	}
	if (self->projectile != NULL)
	{
		projectile = entity_new();
		if (projectile != NULL)
		{
			entity_copy_prefab(projectile, self->projectile);
			projectile->position = self->position;
			projectile->velocity = self->forward;
			projectile->parent = self;
			projectile->damage = projectile->damage * ((double)self->charge / self->maxCharge);
			vector2d_set_magnitude(&projectile->velocity, self->shotSpeed * ((double)self->charge / self->maxCharge));
			self->lastShot = self->timer;
		}
	}
	else
	{
		slog("no projectile to spawn");
	}
}

void player_pull(Entity *self, Vector2D eyePos)
{
	Entity *nearestStone;
	Entity *nearestCorpse;
	Sonar *sonar;
	Vector2D direction, diff, diff2;
	if (self->holding == NULL)
	{
		direction = vector2d_rotate(self->forward, self->fov * GF2D_PI / -360);
		nearestStone = entity_closest_in_sight_by_layer(self, 6, eyePos, direction);
		nearestCorpse = entity_closest_in_sight_by_layer(self, 5, eyePos, direction);
		if (nearestStone == NULL)
			nearestStone = nearestCorpse;
		if (nearestStone != NULL)
		{
			vector2d_sub(diff, eyePos, nearestStone->position);
			if (nearestCorpse != NULL)
			{
				vector2d_sub(diff2, eyePos, nearestCorpse->position);
				if (vector2d_magnitude_squared(diff2) < vector2d_magnitude_squared(diff))
					nearestStone = nearestCorpse;
			}
			if (vector2d_magnitude_squared(diff) < 2500)
			{
				self->holding = nearestStone;
				nearestStone->lastPosition = nearestStone->position;
				nearestStone->position.x += self->velocity.x;
				nearestStone->position.y += self->velocity.y;
				if (self->holding->layer == 6)
				{
					sonar = sonar_new();
					if (!sonar)
					{
						slog("Could not spawn a sonar!");
						return;
					}
					sonar->radius = 80;
					sonar->center = self->holding->position;
					sonar->noise = 100;
					self->moveSpeed *= 0.5;
				}
			}
		}
	}
	else
	{
		vector2d_sub(diff, eyePos, self->holding->position);
		if (vector2d_magnitude_squared(diff) < 5000)
		{
			self->holding->lastPosition = self->holding->position;
			self->holding->position.x += self->velocity.x;
			self->holding->position.y += self->velocity.y;

			if (self->timer % 60 == 0)
			{
				sonar = sonar_new();
				if (!sonar)
				{
					slog("Could not spawn a sonar!");
					return;
				}
				sonar->radius = 600;
				sonar->center = self->holding->position;
				sonar->noise = 100;
			}
		}
		else
		{
			if (self->holding->layer == 6)
				self->moveSpeed *= 2;
			self->holding = NULL;
		}
	}
}

void player_update(Entity *self)
{
	Entity *projectile;
	int mx, my;
	Vector2D direction;
	Vector2D eyePos;
	Entity *nearestStone;
	double dist;
	Vector2D diff;

	vector2d_add(eyePos, self->position, self->eyePos);

	if (input_get_button(INPUT_HURT_SELF) == true)
		player_take_damage(self, 1);

	//direction = self->forward;
	//direction = vector2d_rotate(direction, self->fov * GF2D_PI / -360);
	self->lastPosition = self->position;
	/*if (!controller)
	{
		SDL_GetMouseState(&mx, &my);
		self->position = vector2d(mx, my);
	}
	else
	{*/
		self->velocity.x = self->moveSpeed * input_get_axis(INPUT_AXIS_MOVE_X) / (double)10000;
		self->velocity.y = self->moveSpeed * input_get_axis(INPUT_AXIS_MOVE_Y) / (double)10000;
		self->forward = vector2d(input_get_axis(INPUT_AXIS_FWD_X), input_get_axis(INPUT_AXIS_FWD_Y));
		if (vector2d_magnitude_squared(self->velocity) < 0.1)
		{
			self->velocity = vector2d(0, 0);
		}
		else
			self->lastForward = self->velocity;
		if (vector2d_magnitude_squared(self->forward) < 40000000)
		{
			self->forward = self->lastForward;
		}
		vector2d_set_magnitude(&self->forward, self->maxSight);
		//if (SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) > 1000)
		if (input_get_button(INPUT_BUTTON_CHARGE))
		{
			self->hasReleased = false;
			self->charge += 1;
			if (self->charge > self->maxCharge)
				self->charge = self->maxCharge;
		}
		else
		{
			if (!self->hasReleased)
			{
				player_shoot(self);
			}
			self->hasReleased = true;
			self->charge = 0;
		}
		if (input_get_button(INPUT_BUTTON_MELEE))
		{
			if (self->releasedX)
				player_eat(self, eyePos);
			self->releasedX = false;
		}
		else
			self->releasedX = true;
		if (input_get_button(INPUT_BUTTON_PULL))
		{
			player_pull(self, eyePos);
		}
		else if (self->holding != NULL)
		{
			if (self->holding->layer == 6)
				self->moveSpeed *= 2;
			self->holding = NULL;
		}
	//}
	//draw_line_of_sight(self, 1, self->fov, direction, vector4d(255,255,70,255), 10, eyePos);
	self->timer += 1;
}



void player_touch(Entity *self, Entity *other)
{
	Vector2D selfCenter;
	Vector2D otherCenter;
	Vector2D centerDiff;
	if (other->layer == 1)
	{
		//self->position = self->lastPosition;
		//slog("We're hitting something hard");
		// push us out from the box collider we're hitting
		selfCenter = vector2d(self->position.x + self->coll->width / 2, self->position.y + self->coll->height / 2);
		otherCenter = vector2d(other->position.x + other->coll->width / 2, other->position.y + other->coll->height / 2);
		//gf2d_draw_line(selfCenter, otherCenter, vector4d(255,255,255,255));
		vector2d_sub(centerDiff, selfCenter, otherCenter);

		if (centerDiff.x > 0)
		{
			if (centerDiff.y > 0)
			{
				if ((centerDiff.y / centerDiff.x) < (other->coll->height / other->coll->width))
				{
					// push us out toward the right side
					self->position.x = other->position.x + other->coll->width;
					//slog("CASE A");
				}
				else
				{
					// push us out the bottom
					self->position.y = other->coll->height + other->position.y;
					//slog("CASE B");
				}
			}
			else
			{
				if ((centerDiff.y / centerDiff.x) * -1 < (other->coll->height / other->coll->width))
				{
					// push us out toward the right side
					self->position.x = other->position.x + other->coll->width;
					//slog("CASE C");
				}
				else
				{
					// push us out the top
					self->position.y = other->position.y - self->coll->height;
					//slog("CASE D");
				}
			}
		}
		else
		{
			if (centerDiff.y > 0)
			{
				if ((centerDiff.y / centerDiff.x) * -1  < (other->coll->height / other->coll->width))
				{
					// push us out the left side
					self->position.x = other->position.x - self->coll->width;
					//slog("CASE E");
				}
				else
				{
					// push us out the bottom
					self->position.y = other->position.y + other->coll->height;
					//slog("CASE F");
				}
			}
			else
			{
				if ((centerDiff.y / centerDiff.x) < (other->coll->height / other->coll->width))
				{
					// push us out the left side
					self->position.x = other->position.x - self->coll->width;
					//slog("CASE G");
				}
				else
				{
					// push us out the top
					self->position.y = other->position.y - self->coll->height;
					//slog("CASE H");
				}
			}
		}
	}
}

void player_take_damage(Entity *self, int damage)
{
	if (!self) return;
	if (self->timer - self->lastHit > 60)
	{
		self->colorShift.x -= 5;
		self->lastHit = self->timer;
		sound_play(self->ouch, 0, self->ouch->volume, self->ouch->defaultChannel, 0, SOUND_T);

		self->health -= damage;
		//slog("player health: %i %i", player->maxHealth, player->health);
		if (self->health <= 0)
			self->die(self);
	}
}

void player_die(Entity *self)
{
	entity_free(self);
	set_player(NULL);
	slog("player dead");
}