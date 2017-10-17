#include <SDL.h>
#include "gf2d_vector.h"
#include "gf2d_sprite.h"
#include "collision.h"
#include "raycast.h"
#include "audio.h"

#ifndef _ENTITY_H_
#define _ENTITY_H_

typedef struct Entity_s
{
	bool inUse;
	int layer;
	char name[255];

	int health;
	int damage;

	int killCount;
	bool releasedX;

	int timer;
	Vector2D position;
	Vector2D velocity;
	Vector2D lastPosition;

	Vector2D scale;
	Vector2D spriteOffset;

	double moveSpeed;
	double turnSpeed;
	Vector2D forward;
	Vector2D lastForward;
	Vector2D eyePos;
	double fov;
	double maxSight;

	struct Entity_s *parent;

	struct Entity_s *projectile;
	int reload;
	int lastShot;
	int charge;
	int maxCharge;
	double shotSpeed;
	bool hasReleased;

	struct Entity_s *holding;

	struct Entity_s *corpse;
	struct Entity_s *stone;

	int alert;
	int currentDestination;
	int numPatrol;
	Vector2D patrol[4];

	bool seeThePlayer;
	Vector2D lastKnownPosition;

	Vector2D hunt[5];
	double huntRadius;
	int huntLoops;

	Vector2D retreat;
	bool wasRetreating;
	Vector2D behindMe;

	BoxCollider *coll;
	void(*update)(struct Entity_s *self);
	void(*touch)(struct Entity_s *self, struct Entity_s *other);
	void(*init)(struct Entity_s *self);
	void(*take_damage)(struct Entity_s *self, int damage);
	void(*die)(struct Entity_s *self);
	void(*turn_to_stone)(struct Entity_s *self);
	
	Sound *ouch;
	int lastHit;
	Sprite *sprite;
	float frame;
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

Entity *entity_closest_in_sight_by_layer(Entity *self, int layer, Vector2D eyePos, Vector2D direction);

bool entity_can_i_see_you(Entity *self, Entity *other, Vector2D eyePos, Vector2D direction);

void draw_line_of_sight(Entity *self, int layer, double fov, Vector2D direction, Vector4D color, double degreesToCheck, Vector2D eyePos);

RaycastHit *raycast_through_all_entities(Vector2D start, Vector2D direction, int layer);

void entity_draw(Entity *ent);

void entity_draw_all();

void entity_system_close();

void entity_copy_prefab(Entity *ent, Entity *prefab);
#endif

#pragma once