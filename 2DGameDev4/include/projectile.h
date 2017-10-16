#ifndef _PROJECTILE_H_
#define _PROJECTILE_H_

#include "entity.h"
#include "collision.h"
#include "gf2d_vector.h"

void arrow_update(Entity *self);

void arrow_touch(Entity *self, Entity *other);

void arrow_init(Entity *self);
#endif
#pragma once
