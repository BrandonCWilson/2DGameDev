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
		/*gf2d_draw_line(
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
			vector4d(255, 255, 255, 255));*/
	}
}

int FindLineCircleIntersections(
	float cx, float cy, float radius,
	Vector2D point1, Vector2D point2,
	Vector2D *intersection1, Vector2D *intersection2)
{
	float dx, dy, A, B, C, det, t;

	dx = point2.x - point1.x;
	dy = point2.y - point1.y;

	A = dx * dx + dy * dy;
	B = 2 * (dx * (point1.x - cx) + dy * (point1.y - cy));
	C = (point1.x - cx) * (point1.x - cx) +
		(point1.y - cy) * (point1.y - cy) -
		radius * radius;

	det = B * B - 4 * A * C;
	if ((A <= 0.0000001) || (det < 0))
	{
		// No real solutions.
		slog("no solution");
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

void connect_to_walls(RaycastHit *hit, Entity *self, float fwdMag, Vector2D forward, PriorityQueueList *pts, float initialAngle, float endAngle)
{
	Vector2D hitdiff, dir, intersection1, intersection2;
	float tmpAngle;
	if (hit->other != NULL)
	{
		if ((hit->hitpoint.x > hit->other->corners[3].x) && (hit->hitpoint.x < hit->other->corners[0].x))
		{
			if (self->position.x >= hit->hitpoint.x)
			{
				FindLineCircleIntersections(
					self->position.x, self->position.y, fwdMag,
					hit->hitpoint, vector2d(hit->hitpoint.x - 200, hit->hitpoint.y),
					&intersection1, &intersection2);
			}
			else
			{
				FindLineCircleIntersections(
					self->position.x, self->position.y, fwdMag,
					hit->hitpoint, vector2d(hit->hitpoint.x + 200, hit->hitpoint.y),
					&intersection1, &intersection2);
			}
			hit = raycast_through_all_entities(self->position, intersection1, 1); if (!hit)
				return;
			vector2d_sub(dir, intersection1, self->position);
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
				vector2d_sub(hitdiff, intersection1, self->position);
				pqlist_insert(pts, hit, acos(vector2d_dot_product(hitdiff, forward) / (vector2d_magnitude(hitdiff)*fwdMag)));
			}
			else
			{
				raycasthit_free(hit);
			}
		}
		else
		{
			if (self->position.y >= hit->hitpoint.y)
			{
				FindLineCircleIntersections(
					self->position.x, self->position.y, fwdMag,
					hit->hitpoint, vector2d(hit->hitpoint.x, hit->hitpoint.y - 200),
					&intersection1, &intersection2);
			}
			else
			{
				FindLineCircleIntersections(
					self->position.x, self->position.y, fwdMag,
					hit->hitpoint, vector2d(hit->hitpoint.x, hit->hitpoint.y + 200),
					&intersection1, &intersection2);
			}
			hit = raycast_through_all_entities(self->position, intersection1, 1); if (!hit)
				return;
			vector2d_sub(dir, intersection1, self->position);
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
				vector2d_sub(hitdiff, intersection1, self->position);
				pqlist_insert(pts, hit, acos(vector2d_dot_product(hitdiff, forward) / (vector2d_magnitude(hitdiff)*fwdMag)));
			}
			else
			{
				raycasthit_free(hit);
			}
		}
	}
}

void draw_line_of_sight_2(Entity *self, int layer, double fov, Vector2D forward, Vector4D color, double precision)
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
	vector2d_add(end, self->position, dir);
	hit = raycast_through_all_entities(self->position, end, layer);
	if (!hit) return;
	vector2d_sub(hitdiff, hit->hitpoint, self->position);
	initialHitpoint = hit->hitpoint;
	pqlist_insert(pts, hit, acos(vector2d_dot_product(hitdiff, forward) / (vector2d_magnitude(hitdiff)*fwdMag)));
	connect_to_walls(hit, self, fwdMag, forward, pts, initialAngle, endAngle);
	vector2d_add(end, self->position, rotated);
	hit = raycast_through_all_entities(self->position, end, layer);
	if (!hit) return;
	vector2d_sub(hitdiff, hit->hitpoint, self->position);
	endHitpoint = hit->hitpoint;
	pqlist_insert(pts, hit, acos(vector2d_dot_product(hitdiff, forward) / (vector2d_magnitude(hitdiff)*fwdMag)));
	connect_to_walls(hit, self, fwdMag, forward, pts, initialAngle, endAngle);
	for (tmpAngle = 0; tmpAngle < fov * (GF2D_PI / 180); tmpAngle += (precision * GF2D_PI / 180))
	{
		dir = vector2d_rotate(forward, tmpAngle);
		vector2d_add(end, dir, self->position);
		hit = raycast_through_all_entities(self->position, end, layer);
		if (!hit)
			continue;
		vector2d_sub(hitdiff, hit->hitpoint, self->position);
		pqlist_insert(pts, hit, tmpAngle);
		connect_to_walls(hit, self, fwdMag, forward, pts, initialAngle, endAngle);
	}
	for (i = 0; i < entity_manager.max_entities; i++)
	{
		if (!entity_manager.ent_list[i].inUse) continue;
		if (!entity_manager.ent_list[i].coll) continue;
		if (entity_manager.ent_list[i].layer != layer) continue;
		for (j = 0; j < 4; j++)
		{
			vector2d_sub(dir, entity_manager.ent_list[i].coll->corners[j], self->position);
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
			vector2d_add(end, dir, self->position);
			hit = raycast_through_all_entities(self->position, end, layer);
			if (!hit)
				continue;
			vector2d_sub(hitdiff, hit->hitpoint, self->position);
			pqlist_insert(pts, hit, acos(vector2d_dot_product(hitdiff, forward) / (vector2d_magnitude(hitdiff)*fwdMag)));
			connect_to_walls(hit, self, fwdMag, forward, pts, initialAngle, endAngle);
			if ((hit->hitpoint.x == entity_manager.ent_list[i].coll->corners[j].x)
				&& (hit->hitpoint.y == entity_manager.ent_list[i].coll->corners[j].y))
			{
				vector2d_set_magnitude(&dir, fwdMag);
				dir = vector2d_rotate(dir, 0.01);
				vector2d_add(end, dir, self->position);
				hit = raycast_through_all_entities(self->position, end, layer);
				if (!hit) continue;
				vector2d_sub(hitdiff, hit->hitpoint, self->position);
				pqlist_insert(pts, hit, acos(vector2d_dot_product(hitdiff, forward) / (vector2d_magnitude(hitdiff)*fwdMag)));
				connect_to_walls(hit, self, fwdMag, forward, pts, initialAngle, endAngle);
				dir = vector2d_rotate(dir, -0.02);
				vector2d_add(end, dir, self->position);
				hit = raycast_through_all_entities(self->position, end, layer);
				if (!hit) continue;
				vector2d_sub(hitdiff, hit->hitpoint, self->position);
				pqlist_insert(pts, hit, acos(vector2d_dot_product(hitdiff, forward) / (vector2d_magnitude(hitdiff)*fwdMag)));
				connect_to_walls(hit, self, fwdMag, forward, pts, initialAngle, endAngle);
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
	x[0] = self->position.x;
	y[0] = self->position.y;
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

void draw_line_of_sight(Entity *self, int layer, double fov, Vector2D forward, Vector4D color, double precision)
{
	draw_line_of_sight_2(self, layer, fov, forward, color, precision);
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
	for (i = 0; i < entity_manager.max_entities; i++)
	{
		if (entity_manager.ent_list[i].inUse)
		{

			vector2d_add(entity_manager.ent_list[i].position, entity_manager.ent_list[i].position, entity_manager.ent_list[i].velocity);

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
				if (entity_manager.ent_list[i].frame > 3.0)entity_manager.ent_list[i].frame = 0;
			}
		}

	}
}

void entity_draw(Entity *ent)
{
	if (!ent)
		return;

	gf2d_sprite_draw(
		ent->sprite,
		ent->position,
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
	slog("Entity system closed");
}