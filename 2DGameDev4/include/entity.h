#include <SDL.h>
#include "gf2d_vector.h"
#include "gf2d_sprite.h"
#include "collision.h"
#include "raycast.h"

#ifndef _ENTITY_H_
#define _ENTITY_H_

typedef struct Entity_s
{
	bool inUse;
	int layer;
	char name[255];
	int timer;
	Vector2D position;
	Vector2D scale;
	BoxCollider *coll;
	void(*update)(struct Entity_s *self);
	Sprite *sprite;
	float frame;
	Vector2D velocity;
	Vector4D colorShift;
} Entity;

/**
*@brief initializes the entity manager
*@param max the number of entities the system will handle at once
*/
void entity_system_init(Uint32 max);

/**
*@brief creates a new active entity
*/
Entity *entity_new();

void entity_delete(Entity *ent);

void entity_free(Entity *ent);

void entity_update_all();

void draw_line_of_sight(Entity *self, int layer, double fov);

RaycastHit *raycast_through_all_entities(Vector2D start, Vector2D direction, int layer);

void entity_draw(Entity *ent);

void entity_draw_all();

void entity_system_close();
#endif

#pragma once