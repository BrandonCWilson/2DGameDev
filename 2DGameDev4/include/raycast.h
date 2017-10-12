#ifndef _RAYCAST_H_
#define _RAYCAST_H_

#include "gf2d_vector.h"
#include "collision.h"

typedef struct RaycastHit_S
{
	Vector2D hitpoint;
	BoxCollider *other;
} RaycastHit;

void raycasthit_free(RaycastHit *hit);

RaycastHit *raycasthit_new();

RaycastHit *raycast(Vector2D start1, Vector2D dir1, Vector2D start2, Vector2D dir2);
#endif
