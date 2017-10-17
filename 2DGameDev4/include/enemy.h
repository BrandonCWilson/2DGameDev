#ifndef _ENEMY_H_
#define _ENEMY_H_

#include "entity.h"
#include "player.h"
#include "gf2d_vector.h"

void archer_update(Entity *self);

void archer_init(Entity *self);

void archer_touch(Entity *self, Entity *other);

void archer_take_damage(Entity *self, int damage);

void archer_die(Entity *self);

void archer_turn_to_stone(Entity *self);

void stone_touch(Entity *self, Entity *other);

#endif