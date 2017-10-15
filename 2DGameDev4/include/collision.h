#ifndef _COLLISION_H_
#define _COLLISION_H_

#include "gf2d_vector.h"

typedef struct BoxCollider_S
{
	void *parent;
	double width;
	double height;
	Vector2D corners[4];
} BoxCollider;

typedef struct Collision_S
{
	BoxCollider *a;
	BoxCollider *b;
} Collision;

BoxCollider *box_collider_new();

void box_collider_free(BoxCollider *box);
#endif
