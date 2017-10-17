#include "entity.h"

#ifndef _PLAYER_H_
#define _PLAYER_H_

void player_init(Entity *self);

void player_update(Entity *self);

void player_touch(Entity *self, Entity *other);

void player_take_damage(Entity *self, int damage);

void player_die(Entity *self);

Entity *get_player();
#endif