#include "entity.h"

#ifndef _PLAYER_H_
#define _PLAYER_H_

void player_init(Entity *self);

void player_update(Entity *self);

void player_touch(Entity *self, Entity *other);

Entity *get_player();
#endif