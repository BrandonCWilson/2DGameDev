#include "entity.h"
#include "simple_logger.h"
#include "priority_queue.h"
#include "raycast.h"
#include "gf2d_draw.h"
#include <float.h>
#include <SDL2_gfxPrimitives.h>
#include "gf2d_graphics.h"

typedef struct
{
	Uint32 max_entities;
	Entity * ent_list;
}EntManager;

static EntManager entity_manager;

void entity_copy_prefab(Entity *ent, Entity *prefab)
{
	if (!ent || !prefab)
		return NULL;
	if (prefab->update != NULL)
		ent->update = prefab->update;
	if (prefab->sprite != NULL)
	{
		ent->sprite = prefab->sprite;
		ent->colorShift = vector4d(255, 255, 255, 255);
	}
	ent->layer = prefab->layer;
	if (prefab->touch != NULL)
		ent->touch = prefab->touch;
	if (prefab->coll != NULL)
	{
		ent->coll = box_collider_new();
		ent->coll->width = prefab->coll->width;
		ent->coll->height = prefab->coll->height;
		ent->coll->parent = ent;
	}
	if (prefab->ouch != NULL)
	{
		ent->ouch = prefab->ouch;
	}
	if (prefab->init != NULL)
		ent->init = prefab->init;
	if (prefab->projectile != NULL)
		ent->projectile = prefab->projectile;
	if (prefab->corpse != NULL)
		ent->corpse = prefab->corpse;
	if (prefab->take_damage != NULL)
		ent->take_damage = prefab->take_damage;
	if (prefab->die != NULL)
		ent->die = prefab->die;
	ent->health = prefab->health;
	ent->scale = prefab->scale;
	ent->spriteOffset = prefab->spriteOffset;
	ent->fov = prefab->fov;
	ent->damage = prefab->damage;
	ent->maxSight = prefab->maxSight;
	ent->forward = vector2d(1, 0);
	ent->moveSpeed = prefab->moveSpeed;
	ent->turnSpeed = prefab->turnSpeed;
	ent->huntRadius = prefab->huntRadius;
	ent->reload = prefab->reload;
	ent->maxCharge = prefab->maxCharge;
	ent->shotSpeed = prefab->shotSpeed;
	if (prefab->turn_to_stone != NULL)
		ent->turn_to_stone = prefab->turn_to_stone;
	if (prefab->stone != NULL)
		ent->stone = prefab->stone;
}

bool entity_can_i_see_you(Entity *self, Entity *other, Vector2D eyePos, Vector2D direction)
{
	int i;
	double tmpAngle;
	double distPlayer, distMAX;
	float initialAngle, endAngle;
	Vector2D dir, end, rotated, longestColl;
	RaycastHit *hit;
	if (!self || !other)
		return false;
	if (!other->coll)
		return false;
	dir = direction;
	rotated = vector2d_rotate(dir, self->fov * GF2D_PI / 180);
	initialAngle = vector2d_angle(dir); endAngle = vector2d_angle(rotated);
	if (initialAngle < 0)
		initialAngle = initialAngle + 360;
	if (endAngle < 0)
		endAngle = endAngle + 360;
	if (endAngle - initialAngle < 0)
	{
		endAngle = endAngle + 360;
	}
	end = vector2d(other->position.x + other->coll->width / 2, other->position.y + other->coll->height / 2);
	vector2d_sub(dir, end, eyePos);
	vector2d_sub(longestColl, other->coll->corners[2], other->coll->corners[0]);
	if (self->maxSight + vector2d_magnitude(longestColl) / 2 < vector2d_magnitude(dir))
		return 0;
	tmpAngle = vector2d_angle(dir);
	if (tmpAngle < 0)
		tmpAngle += 360;
	if (!((!((tmpAngle <= initialAngle) || (tmpAngle >= endAngle)))
		|| ((360 < endAngle) && (tmpAngle <= endAngle - 360))))
	{
		return 0;
	}
	vector2d_add(end, dir, eyePos);
	// check for walls blocking vision
	hit = raycast_through_all_entities(eyePos, end, 1);
	if (!hit)
		return 0;
	if (hit->other != NULL)
	{
		//gf2d_draw_line(eyePos, hit->hitpoint, vector4d(255, 0, 0, 255));
		return 0;
	}
	//gf2d_draw_line(eyePos, hit->hitpoint, vector4d(0, 255, 0, 255));
	raycasthit_free(hit);
	// check if we can see the ent on its own layer
	hit = raycast_through_all_entities(eyePos, end, other->layer);
	if (!hit)
		return 0;
	if (hit->other == NULL)
	{
		raycasthit_free(hit);
		return 0;
	}
	if (hit->other->parent == other)
		return 1;
	raycasthit_free(hit);
	return 0;
}

Entity *entity_closest_in_sight_by_layer(Entity *self, int layer, Vector2D eyePos, Vector2D direction)
{
	int i;
	Entity *closest = NULL;
	Vector2D diff;
	double minDist = DBL_MAX;
	for (i = 0; i < entity_manager.max_entities; i++)
	{
		if (!entity_manager.ent_list[i].inUse)
			continue;
		if (entity_manager.ent_list[i].layer != layer)
			continue;
		if (&entity_manager.ent_list[i] == self)
			continue;
		if (&entity_manager.ent_list[i].coll == NULL)
			continue;
		if (entity_can_i_see_you(self, &entity_manager.ent_list[i], eyePos, direction) == 1)
		{
			vector2d_sub(diff, 
				vector2d(entity_manager.ent_list[i].position.x + entity_manager.ent_list[i].coll->width / 2,
					entity_manager.ent_list[i].position.y + entity_manager.ent_list[i].coll->height / 2),
				eyePos);
			if (vector2d_magnitude_squared(diff) < minDist)
			{
				closest = &entity_manager.ent_list[i];
				minDist = vector2d_magnitude_squared(diff);
			}
		}
	}
	return closest;
}

void entity_update_all_colliders()
{
	int i, j;

	for (i = 0; i < entity_manager.max_entities; i++)
	{
		if (entity_manager.ent_list[i].inUse == false)
			continue;
		if (entity_manager.ent_list[i].coll == NULL)
			continue;
		for (j = 0; j < 4; j++)
		{
			vector2d_add(entity_manager.ent_list[i].coll->corners[j],
				entity_manager.ent_list[i].position,
				vector2d(
					entity_manager.ent_list[i].coll->width * ((j > 1) ? 0 : 1),
					entity_manager.ent_list[i].coll->height * ((j % 3 > 0) ? 0 : 1))
				);
		}
		// collider line for debugging purposes
		gf2d_draw_line(
			entity_manager.ent_list[i].position,
			entity_manager.ent_list[i].coll->corners[0],
			vector4d(0, 255, 255, 255));
		gf2d_draw_line(
			entity_manager.ent_list[i].position,
			entity_manager.ent_list[i].coll->corners[1],
			vector4d(255, 0, 255, 255));
		gf2d_draw_line(
			entity_manager.ent_list[i].position,
			entity_manager.ent_list[i].coll->corners[2],
			vector4d(255, 255, 0, 255));
		gf2d_draw_line(
			entity_manager.ent_list[i].position,
			entity_manager.ent_list[i].coll->corners[3],
			vector4d(255, 255, 255, 255));
	}
}

int FindLineCircleIntersections(
	double cx, double cy, double radius,
	Vector2D point1, Vector2D point2,
	Vector2D *intersection1, Vector2D *intersection2)
{
	double dx, dy, A, B, C, det, t;

	dx = point2.x - point1.x;
	dy = point2.y - point1.y;

	A = dx * dx + dy * dy;
	B = 2 * (dx * (point1.x - cx) + dy * (point1.y - cy));
	C = (point1.x - cx) * (point1.x - cx) +
		(point1.y - cy) * (point1.y - cy) -
		radius * radius;

	det = B * B - 4 * A * C;
	if ((A <= 0) || (det < 0))
	{
		// No real solutions.
		slog("no solution");
		//slog("%lf %lf", A, det);
		return 0;
	}
	else if (det == 0)
	{
		// One solution.
		t = -B / (2 * A);
		*intersection1 = vector2d(point1.x - t * dx, point1.y + t * dy);
		return 1;
	}
	else
	{
		// Two solutions.
		t = (float)((-B + sqrt(det)) / (2 * A));
		*intersection1 = vector2d(point1.x + t * dx, point1.y + t * dy);
		t = (float)((-B - sqrt(det)) / (2 * A));
		*intersection2 = vector2d(point1.x - t * dx, point1.y + t * dy);
		return 2;
	}
}

int get_box_overlap(BoxCollider *a, BoxCollider *b)
{
	BoxCollider *AB, *CD;
	float A, B, C, D;
	if (a->corners[3].x <= b->corners[3].x)
	{
		AB = a;
		CD = b;
	}
	else
	{
		AB = b;
		CD = a;
	}
	if (!((AB->corners[0].x - CD->corners[3].x > 0) && (CD->corners[0].x - AB->corners[3].x > 0)))
		return 0;
	if (a->corners[2].y <= b->corners[2].y)
	{
		AB = a;
		CD = b;
	}
	else
	{
		AB = b;
		CD = a;
	}
	if (!((AB->corners[3].y - CD->corners[2].y > 0) && (CD->corners[3].y - AB->corners[2].y > 0)))
		return 0;
	return 1;
}

void check_box_collisions(int layer1, int layer2)
{
	int i, j;
	for (i = 0; i < entity_manager.max_entities; i++)
	{
		if (entity_manager.ent_list[i].inUse == false)
			continue;
		if (entity_manager.ent_list[i].coll == NULL)
			continue;
		if (entity_manager.ent_list[i].layer == layer1)
		{
			for (j = i + 1; j < entity_manager.max_entities; j++)
			{
				if (entity_manager.ent_list[j].inUse == false)
					continue;
				if (entity_manager.ent_list[j].coll == NULL)
					continue;
				if (entity_manager.ent_list[j].layer == layer2)
				{
					if (get_box_overlap(entity_manager.ent_list[i].coll, entity_manager.ent_list[j].coll) == 1)
					{
						if (entity_manager.ent_list[i].touch != NULL)
							entity_manager.ent_list[i].touch(&entity_manager.ent_list[i], &entity_manager.ent_list[j]);
						if (entity_manager.ent_list[j].touch != NULL)
							entity_manager.ent_list[j].touch(&entity_manager.ent_list[j], &entity_manager.ent_list[i]);
					}
				}
			}
		}
		else if (entity_manager.ent_list[i].layer == layer2)
		{
			for (j = i + 1; j < entity_manager.max_entities; j++)
			{
				if (entity_manager.ent_list[j].inUse == false)
					continue;
				if (entity_manager.ent_list[j].coll == NULL)
					continue; 
				if (entity_manager.ent_list[j].layer == layer1)
				{
					if (get_box_overlap(entity_manager.ent_list[i].coll, entity_manager.ent_list[j].coll) == 1)
					{
						if (entity_manager.ent_list[i].touch != NULL)
							entity_manager.ent_list[i].touch(&entity_manager.ent_list[i], &entity_manager.ent_list[j]);
						if (entity_manager.ent_list[j].touch != NULL)
							entity_manager.ent_list[j].touch(&entity_manager.ent_list[j], &entity_manager.ent_list[i]);
					}
				}
			}
		}
	}
}

void connect_to_walls(RaycastHit *hit, Entity *self, float fwdMag, Vector2D forward, PriorityQueueList *pts, float initialAngle, float endAngle, Vector2D eyePos)
{
	Vector2D hitdiff, dir, intersection1, intersection2;
	float tmpAngle;
	if (hit->other != NULL)
	{
		if ((hit->hitpoint.x > hit->other->corners[3].x) && (hit->hitpoint.x < hit->other->corners[0].x))
		{
			if (eyePos.x >= hit->hitpoint.x)
			{
				if (FindLineCircleIntersections(
					eyePos.x, eyePos.y, fwdMag,
					hit->hitpoint, vector2d(hit->hitpoint.x - fwdMag * 2, hit->hitpoint.y),
					&intersection1, &intersection2) == 0)
				{
					return;
				}
			}
			else
			{
				if (FindLineCircleIntersections(
					eyePos.x, eyePos.y, fwdMag,
					hit->hitpoint, vector2d(hit->hitpoint.x + fwdMag * 2, hit->hitpoint.y),
					&intersection1, &intersection2) == 0)
				{
					return;
				}
			}
			hit = raycast_through_all_entities(eyePos, intersection1, 1); if (!hit)
				return;
			vector2d_sub(dir, intersection1, eyePos);
			tmpAngle = vector2d_angle(dir);
			if (tmpAngle < 0)
				tmpAngle += 360;
			if (((!((tmpAngle <= initialAngle) || (tmpAngle >= endAngle)))
				|| ((360 < endAngle) && (tmpAngle < endAngle - 360)))
				&&
				(((hit->hitpoint.x - intersection1.x < 0.0005) && (hit->hitpoint.x - intersection1.x > -0.0005))
					&& ((hit->hitpoint.y - intersection1.y < 0.0005) && (hit->hitpoint.y - intersection1.y > -0.0005))))
			{
				hit->hitpoint = intersection1;
				vector2d_sub(hitdiff, intersection1, eyePos);
				pqlist_insert(pts, hit, acos(vector2d_dot_product(hitdiff, forward) / (vector2d_magnitude(hitdiff)*fwdMag)));
			}
			else
			{
				raycasthit_free(hit);
			}
		}
		else
		{
			if (eyePos.y >= hit->hitpoint.y)
			{
				if (FindLineCircleIntersections(
					eyePos.x, eyePos.y, fwdMag,
					hit->hitpoint, vector2d(hit->hitpoint.x, hit->hitpoint.y - fwdMag * 2),
					&intersection1, &intersection2) == 0)
					return;
			}
			else
			{
				if (FindLineCircleIntersections(
					eyePos.x, eyePos.y, fwdMag,
					hit->hitpoint, vector2d(hit->hitpoint.x, hit->hitpoint.y + fwdMag * 2),
					&intersection1, &intersection2) == 0)
					return;
			}

			hit = raycast_through_all_entities(eyePos, intersection1, 1); if (!hit)
				return;
			vector2d_sub(dir, intersection1, eyePos);
			tmpAngle = vector2d_angle(dir);
			if (tmpAngle < 0)
				tmpAngle += 360;
			if (((!((tmpAngle <= initialAngle) || (tmpAngle >= endAngle)))
				|| ((360 < endAngle) && (tmpAngle < endAngle - 360)))
				&&
				(((hit->hitpoint.x - intersection1.x < 0.0005) && (hit->hitpoint.x - intersection1.x > -0.0005))
					&& ((hit->hitpoint.y - intersection1.y < 0.0005) && (hit->hitpoint.y - intersection1.y > -0.0005))))
			{
				hit->hitpoint = intersection1;
				vector2d_sub(hitdiff, intersection1, eyePos);
				pqlist_insert(pts, hit, acos(vector2d_dot_product(hitdiff, forward) / (vector2d_magnitude(hitdiff)*fwdMag)));
			}
			else
			{
				raycasthit_free(hit);
			}
		}
	}
}

void draw_line_of_sight(Entity *self, int layer, double fov, Vector2D forward, Vector4D color, double precision, Vector2D eyePos)
{
	int i, j, numPoints;
	Sint16 *x, *y;
	Vector2D dir;
	float fwdMagSquared = vector2d_magnitude_squared(forward);
	float fwdMag = vector2d_magnitude(forward);
	Vector2D rotated;
	Vector2D end;
	Vector2D hitdiff;
	float initialAngle, endAngle, tmpAngle;
	float fovcompare;
	Vector2D initialHitpoint, endHitpoint;
	Vector2D intersection1, intersection2;
	Vector2D tangent;
	RaycastHit *hit = NULL;
	PriorityQueueList *pts;
	if (!self) return;
	pts = pqlist_new();
	if (!pts) return;
	// add the start and end of the fov
	dir = forward;
	rotated = vector2d_rotate(dir, fov * GF2D_PI / 180);
	initialAngle = vector2d_angle(dir); endAngle = vector2d_angle(rotated);
	if (initialAngle < 0)
		initialAngle = initialAngle + 360;
	if (endAngle < 0)
		endAngle = endAngle + 360;
	if (endAngle - initialAngle < 0)
	{
		endAngle = endAngle + 360;
	}
	vector2d_add(end, eyePos, dir);
	hit = raycast_through_all_entities(eyePos, end, layer);
	if (!hit) return;
	vector2d_sub(hitdiff, hit->hitpoint, eyePos);
	initialHitpoint = hit->hitpoint;
	pqlist_insert(pts, hit, acos(vector2d_dot_product(hitdiff, forward) / (vector2d_magnitude(hitdiff)*fwdMag)));
	connect_to_walls(hit, self, fwdMag, forward, pts, initialAngle, endAngle, eyePos);
	vector2d_add(end, eyePos, rotated);
	hit = raycast_through_all_entities(eyePos, end, layer);
	if (!hit) return;
	vector2d_sub(hitdiff, hit->hitpoint, eyePos);
	endHitpoint = hit->hitpoint;
	pqlist_insert(pts, hit, acos(vector2d_dot_product(hitdiff, forward) / (vector2d_magnitude(hitdiff)*fwdMag)));
	connect_to_walls(hit, self, fwdMag, forward, pts, initialAngle, endAngle, eyePos);
	for (tmpAngle = 0; tmpAngle < fov * (GF2D_PI / 180); tmpAngle += (precision * GF2D_PI / 180))
	{
		dir = vector2d_rotate(forward, tmpAngle);
		vector2d_add(end, dir, eyePos);
		hit = raycast_through_all_entities(eyePos, end, layer);
		if (!hit)
			continue;
		vector2d_sub(hitdiff, hit->hitpoint, eyePos);
		pqlist_insert(pts, hit, tmpAngle);
		connect_to_walls(hit, self, fwdMag, forward, pts, initialAngle, endAngle, eyePos);
	}
	for (i = 0; i < entity_manager.max_entities; i++)
	{
		if (!entity_manager.ent_list[i].inUse) continue;
		if (!entity_manager.ent_list[i].coll) continue;
		if (entity_manager.ent_list[i].layer != layer) continue;
		for (j = 0; j < 4; j++)
		{
			vector2d_sub(dir, entity_manager.ent_list[i].coll->corners[j], eyePos);
			end = entity_manager.ent_list[i].coll->corners[j];
			if (fwdMagSquared < vector2d_magnitude_squared(dir))
				continue;
			tmpAngle = vector2d_angle(dir);
			if (tmpAngle < 0)
				tmpAngle += 360;
			if (!((!((tmpAngle <= initialAngle) || (tmpAngle >= endAngle)))
				|| ((360 < endAngle) && (tmpAngle <= endAngle - 360))))
			{
				continue;
			}
			vector2d_add(end, dir, eyePos);
			hit = raycast_through_all_entities(eyePos, end, layer);
			if (!hit)
				continue;
			vector2d_sub(hitdiff, hit->hitpoint, eyePos);
			//gf2d_draw_line(eyePos, hit->hitpoint, vector4d(255, 255, 255, 255));
			pqlist_insert(pts, hit, acos(vector2d_dot_product(hitdiff, forward) / (vector2d_magnitude(hitdiff)*fwdMag)));
			connect_to_walls(hit, self, fwdMag, forward, pts, initialAngle, endAngle, eyePos);
			if ((hit->hitpoint.x == entity_manager.ent_list[i].coll->corners[j].x)
				&& (hit->hitpoint.y == entity_manager.ent_list[i].coll->corners[j].y))
			{
				vector2d_set_magnitude(&dir, fwdMag);
				dir = vector2d_rotate(dir, 0.01);
				vector2d_add(end, dir, eyePos);
				hit = raycast_through_all_entities(eyePos, end, layer);
				if (!hit) continue;
				vector2d_sub(hitdiff, hit->hitpoint, eyePos);
				pqlist_insert(pts, hit, acos(vector2d_dot_product(hitdiff, forward) / (vector2d_magnitude(hitdiff)*fwdMag)));
				connect_to_walls(hit, self, fwdMag, forward, pts, initialAngle, endAngle, eyePos);
				dir = vector2d_rotate(dir, -0.02);
				vector2d_add(end, dir, eyePos);
				hit = raycast_through_all_entities(eyePos, end, layer);
				if (!hit) continue;
				vector2d_sub(hitdiff, hit->hitpoint, eyePos);
				pqlist_insert(pts, hit, acos(vector2d_dot_product(hitdiff, forward) / (vector2d_magnitude(hitdiff)*fwdMag)));
				connect_to_walls(hit, self, fwdMag, forward, pts, initialAngle, endAngle, eyePos);
			}
		}
	}
	numPoints = pqlist_get_size(pts) + 2;
	x = (Sint16 *)malloc(sizeof(Sint16)*(numPoints));
	if (!x)
	{
		pqlist_free(pts, raycasthit_free);
		return;
	}
	y = (Sint16 *)malloc(sizeof(Sint16)*(numPoints));
	if (!y)
	{
		pqlist_free(pts, raycasthit_free);
		free(x);
		return;
	}
	memset(x, 0, sizeof(Sint16)*(numPoints)); memset(y, 0, sizeof(Sint16)*(numPoints));
	x[0] = eyePos.x;
	y[0] = eyePos.y;
	x[1] = endHitpoint.x;
	y[1] = endHitpoint.y;
	i = 2;
	for (hit = pqlist_delete_max(pts); hit != NULL; hit = pqlist_delete_max(pts))
	{
		x[i] = (Sint16)hit->hitpoint.x;
		y[i] = (Sint16)hit->hitpoint.y;
		raycasthit_free(hit);
		i++;
	}
	filledPolygonRGBA(
		gf2d_graphics_get_renderer(),
		x,
		y,
		numPoints,
		color.w, color.x, color.y, color.z
		);
	pqlist_free(pts, raycasthit_free);
	free(x);
	free(y);
}

RaycastHit *raycast_through_all_entities(Vector2D start, Vector2D direction, int layer)
{
	RaycastHit *hit = NULL;
	RaycastHit *rtn = NULL;
	float min = FLT_MAX;
	float dist;
	float fwdMag = vector2d_magnitude(direction);
	int i, j;
	Vector2D v1, v2, diff;

	vector2d_sub(diff, direction, start);
	for (i = 0; i < entity_manager.max_entities; i++)
	{
		if (entity_manager.ent_list[i].inUse == false)
			continue;
		if (entity_manager.ent_list[i].layer != layer)
			continue;
		if (entity_manager.ent_list[i].coll != NULL)
		{
			for (j = 0; j < 4; j++)
			{
				v1 = entity_manager.ent_list[i].coll->corners[j];
				if (j == 3)
					v2 = entity_manager.ent_list[i].coll->corners[0];
				else
					v2 = entity_manager.ent_list[i].coll->corners[j + 1];
				if ((((diff.x > 0) && (v1.x <= start.x) && (v2.x <= start.x)) || ((diff.y > 0) && (v1.y <= start.y) && (v2.y <= start.y))) ||
					(((diff.x < 0) && (v1.x >= start.x) && (v2.x >= start.x)) || ((diff.y < 0) && (v1.y >= start.y) && (v2.y >= start.y))))
					continue;
				hit = raycast(start, direction, v1, v2);
				if (!hit)
					continue;
				hit->other = entity_manager.ent_list[i].coll;
				vector2d_sub(v1, start, hit->hitpoint);
				dist = vector2d_magnitude_squared(v1);
				if (dist < min)
				{
					min = dist;
					rtn = hit;
				}
				else
					raycasthit_free(hit);
			}
		}
	}
	if (!rtn)
	{
		rtn = raycasthit_new();
		if (!rtn)
		{
			slog("Unable to allocate a new raycasthit");
			return NULL;
		}
		rtn->hitpoint = direction;
		rtn->other = NULL;
	}
	return rtn;
}

void entity_system_init(Uint32 max)
{
	int i;
	slog("initializing entity system..");
	if (!max)
	{
		slog("cannot initialize an entity system for zero entities!");
		return;
	}
	entity_manager.max_entities = max;
	entity_manager.ent_list = (Entity *)malloc(sizeof(Entity)*max);
	memset(entity_manager.ent_list, 0, sizeof(Entity)*max);

	slog("entity system initialized");
	atexit(entity_system_close);
}

Entity *entity_new()
{
	int i;
	/*search for an unused entity address*/
	for (i = 0; i < entity_manager.max_entities; i++)
	{
		if (!entity_manager.ent_list[i].inUse)
		{
			entity_delete(&entity_manager.ent_list[i]);	// clean up the old data
			entity_manager.ent_list[i].inUse = true;	// set it to inUse
			vector2d_set(entity_manager.ent_list[i].scale, 1, 1);	// don't be ant man
			return &entity_manager.ent_list[i];			// return address of this array element
		}
	}
	slog("error: out of entity addresses");
	return NULL;
}

void entity_delete(Entity *ent)
{
	if (!ent)return;
	if (ent->sprite != NULL)
		gf2d_sprite_free(ent->sprite);
	if (ent->ouch != NULL)
		sound_free(ent->ouch);
	if (ent->coll != NULL)
		box_collider_free(ent->coll);
	if (ent->patharray != NULL)
		patharray_free(ent->patharray);
	memset(ent, 0, sizeof(Entity));	//clean up the data
}

void entity_free(Entity *ent)
{
	if (!ent)return;
	ent->inUse = false;
}

void entity_update_all()
{
	int i;
	entity_update_all_colliders();
	check_box_collisions(2, 1);
	check_box_collisions(4, 1);
	check_box_collisions(4, 2);
	check_box_collisions(4, 3);
	check_box_collisions(6, 1);
	for (i = 0; i < entity_manager.max_entities; i++)
	{
		if (entity_manager.ent_list[i].inUse)
		{
			// handle the entity think functions
			if (!entity_manager.ent_list[i].update)
			{
				// no update function ..
			}
			else
			{
				entity_manager.ent_list[i].update(&entity_manager.ent_list[i]);
			}
			if (!entity_manager.ent_list[i].sprite)
			{
				// no sprite ..
			}
			else
			{
				entity_manager.ent_list[i].frame += 0.1;
				if (entity_manager.ent_list[i].frame > entity_manager.ent_list[i].sprite->frames_per_line)entity_manager.ent_list[i].frame = 0;
			}
			vector2d_add(entity_manager.ent_list[i].position, entity_manager.ent_list[i].position, entity_manager.ent_list[i].velocity);
		}
	}
}

void entity_draw(Entity *ent)
{
	if (!ent)
		return;

	gf2d_sprite_draw(
		ent->sprite,
		vector2d(ent->position.x + ent->spriteOffset.x, ent->position.y + ent->spriteOffset.y),
		&ent->scale,
		NULL,
		NULL,
		NULL,
		&ent->colorShift,
		(int)ent->frame);
}

void entity_draw_all()
{
	int i;
	for (i = 0; i < entity_manager.max_entities; i++)
	{
		if (entity_manager.ent_list[i].inUse)
			entity_draw(&entity_manager.ent_list[i]);
	}
}

void entity_clear_all()
{
	int i;
	for (i = 0; i < entity_manager.max_entities; i++)
	{
		entity_delete(&entity_manager.ent_list[i]);
	}
}

void entity_system_close()
{
	entity_clear_all();
	if (entity_manager.ent_list != NULL)
	{
		free(entity_manager.ent_list);
	}
	entity_manager.ent_list = NULL;
	entity_manager.max_entities = 0;
}