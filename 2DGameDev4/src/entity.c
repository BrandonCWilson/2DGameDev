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

void entitiy_update_all_colliders()
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
		gf2d_draw_line(
			entity_manager.ent_list[i].position,
			entity_manager.ent_list[i].coll->corners[0],
			vector4d(255, 255, 255, 255));
		gf2d_draw_line(
			entity_manager.ent_list[i].position,
			entity_manager.ent_list[i].coll->corners[1],
			vector4d(255, 255, 255, 255));
		gf2d_draw_line(
			entity_manager.ent_list[i].position,
			entity_manager.ent_list[i].coll->corners[2],
			vector4d(255, 255, 255, 255));
		gf2d_draw_line(
			entity_manager.ent_list[i].position,
			entity_manager.ent_list[i].coll->corners[3],
			vector4d(255, 255, 255, 255));
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
		t = (float)((-B +  sqrt(det)) / (2 * A));
		*intersection1 = vector2d (point1.x + t * dx, point1.y + t * dy);
		t = (float)((-B - sqrt(det)) / (2 * A));
		*intersection2 = vector2d(point1.x - t * dx, point1.y + t * dy);
		return 2;
	}
}

// FIXME this function is bloated. break it up into smaller helper functions
void draw_line_of_sight(Entity *self, int layer, double fov, Vector2D forward)
{
	int i, j, numPoints;
	Sint16 *x, *y;
	Vector2D dir;
	float fwdMag = vector2d_magnitude(forward);
	Vector2D rotated;
	Vector2D end;
	Vector2D hitdiff;
	float initialAngle, endAngle, tmpAngle, tmpAngleEnd;
	float fovcompare;
	Vector2D initialHitpoint, endHitpoint;
	RaycastHit *hit = NULL;
	PriorityQueueList *pts;
	bool lastOther = false;
	Vector2D lastDirection = vector2d(0, 0);
	Vector2D circleToLine;
	Vector2D intersection1, intersection2;
	double k1, k2, k3;
	if (!self) return;
	pts = pqlist_new();
	if (!pts) return;
	// add the start and end of the fov
	dir = forward;
	rotated = vector2d_rotate(dir, fov * GF2D_PI / 180);
	slog("fov: %f", acos(vector2d_dot_product(dir, rotated) / (vector2d_magnitude(dir)*vector2d_magnitude(rotated))) * 180 / GF2D_PI);
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
	gf2d_draw_line(self->position, hit->hitpoint, vector4d(255, 0, 0, 255)); 
	initialHitpoint = hit->hitpoint;
	gf2d_draw_circle(initialHitpoint, 20, vector4d(0, 25, 25, 180));
	tmpAngle = vector2d_angle(hitdiff);
	if (tmpAngle < 0)
		tmpAngle += 360;
	pqlist_insert(pts, hit, acos(vector2d_dot_product(hitdiff, forward) / (vector2d_magnitude(hitdiff)*fwdMag)));
	vector2d_add(end, self->position, rotated);
	hit = raycast_through_all_entities(self->position, end, layer);
	if (!hit) return;
	vector2d_sub(hitdiff, hit->hitpoint, self->position);
	gf2d_draw_line(self->position, hit->hitpoint, vector4d(255, 0, 0, 255));
	endHitpoint = hit->hitpoint;
	gf2d_draw_circle(endHitpoint, 20, vector4d(0, 255, 25, 180));
	tmpAngle = vector2d_angle(hitdiff);
	if (tmpAngle < 0)
		tmpAngle += 360;
	pqlist_insert(pts, hit, acos(vector2d_dot_product(hitdiff, forward) / (vector2d_magnitude(hitdiff)*fwdMag)));
	for (i = 0; i < entity_manager.max_entities; i++)
	{
		if (!entity_manager.ent_list[i].inUse) continue;
		if (!entity_manager.ent_list[i].coll) continue;
		if (entity_manager.ent_list[i].layer != layer) continue;
		for (j = 0; j < 4; j++)
		{
			vector2d_sub(dir, entity_manager.ent_list[i].coll->corners[j], self->position);
			end = entity_manager.ent_list[i].coll->corners[j];
			tmpAngle = vector2d_angle(dir);
			if (tmpAngle < 0)
				tmpAngle += 360; 
			if (!((!((tmpAngle <= initialAngle) || (tmpAngle >= endAngle)))
					|| ((360 < endAngle) && (tmpAngle < endAngle - 360))))
			{
				continue;
			}
			if (fwdMag<vector2d_magnitude(dir)) 
				vector2d_set_magnitude(&dir, fwdMag);
			vector2d_add(end, dir, self->position);
			hit = raycast_through_all_entities(self->position, end, layer);
			if (!hit)
				continue;
			vector2d_sub(hitdiff, hit->hitpoint, self->position);
			//gf2d_draw_line(self->position, hit->hitpoint, vector4d(255, 0, 255, 255));
			tmpAngle = vector2d_angle(hitdiff);
			if (tmpAngle - endAngle < 0)
				tmpAngle += 360;
			pqlist_insert(pts, hit, acos(vector2d_dot_product(hitdiff, forward) / (vector2d_magnitude(hitdiff)*fwdMag)));
			if ((hit->hitpoint.x == entity_manager.ent_list[i].coll->corners[j].x)
				&& (hit->hitpoint.y == entity_manager.ent_list[i].coll->corners[j].y))
			{
				end = entity_manager.ent_list[i].coll->corners[j];
				vector2d_set_magnitude(&dir, fwdMag);
				dir = vector2d_rotate(dir, 0.01);
				vector2d_add(end, dir, self->position);
				hit = raycast_through_all_entities(self->position, end, layer);
				if (!hit) continue;
				vector2d_sub(hitdiff, hit->hitpoint, self->position);
				tmpAngle = vector2d_angle(hitdiff);
				if (tmpAngle - endAngle < 0)
					tmpAngle += 360;
				//pqlist_insert(pts, hit, tmpAngle - endAngle);
				pqlist_insert(pts, hit, acos(vector2d_dot_product(hitdiff, forward) / (vector2d_magnitude(hitdiff)*fwdMag)));
				end = entity_manager.ent_list[i].coll->corners[j];
				vector2d_set_magnitude(&dir, fwdMag);
				dir = vector2d_rotate(dir, -0.02);
				vector2d_add(end, dir, self->position);
				hit = raycast_through_all_entities(self->position, end, layer);
				if (!hit) continue;
				vector2d_sub(hitdiff, hit->hitpoint, self->position);
				tmpAngle = vector2d_angle(hitdiff);
				if (tmpAngle - endAngle < 0)
					tmpAngle += 360;
				pqlist_insert(pts, hit, acos(vector2d_dot_product(hitdiff, forward) / (vector2d_magnitude(hitdiff)*fwdMag)));
			}
		}
	}
	numPoints = pqlist_get_size(pts);
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
	i = 0;
	for (hit = pqlist_delete_max(pts); hit != NULL; hit = pqlist_delete_max(pts))
	{
		x[i] = (Sint16)hit->hitpoint.x;
		y[i] = (Sint16)hit->hitpoint.y;
		if ((i > 0))
		{
			//vector2d_sub(dir, hit->hitpoint, self->position);
			if ((hit->other == NULL) && (lastOther))
			{
				vector2d_sub(end, vector2d(x[i], y[i]), self->position);
				vector2d_set_magnitude(&lastDirection, vector2d_magnitude(end) * 2);
				vector2d_add(lastDirection, vector2d(x[i-1],y[i-1]), lastDirection);
				// handle the case where we have to curve into a wall
				FindLineCircleIntersections(self->position.x, self->position.y,
					vector2d_magnitude(end),
					vector2d(x[i - 1], y[i - 1]),
					lastDirection,
					&intersection1,
					&intersection2
					);
				vector2d_sub(dir, intersection1, self->position);
				vector2d_sub(end, vector2d(x[i], y[i]), self->position);
				tmpAngle = vector2d_angle(dir);
				if (tmpAngle < 0)
					tmpAngle += 360;
				tmpAngleEnd = vector2d_angle(end);
				if (tmpAngleEnd < 0)
					tmpAngleEnd += 360;
				if (((tmpAngle - tmpAngleEnd < 180) && (tmpAngleEnd<tmpAngle)))
					filledPieRGBA(
						gf2d_graphics_get_renderer(),
						self->position.x, self->position.y,
						vector2d_magnitude(end),
						tmpAngleEnd,
						tmpAngle,
						150, 255, 10 + i * 5, 70
						);
				if ((tmpAngle - tmpAngleEnd < -180))
					filledPieRGBA(
						gf2d_graphics_get_renderer(),
						self->position.x, self->position.y,
						vector2d_magnitude(end),
						tmpAngleEnd,
						tmpAngle,
						150, 255, 10 + i * 5, 70
						);
				filledTrigonRGBA(
					gf2d_graphics_get_renderer(),
					intersection1.x, intersection1.y,
					x[i - 1], y[i - 1],
					self->position.x, self->position.y,
					150, 255, 10 + i * 5, 70
					);
			}
			else if (hit->other == NULL)
			{
				vector2d_sub(dir, vector2d(x[i - 1], y[i - 1]), self->position);
				vector2d_sub(end, vector2d(x[i], y[i]), self->position);
				tmpAngle = vector2d_angle(dir);
				if (tmpAngle < 0)
					tmpAngle += 360;
				tmpAngleEnd = vector2d_angle(end);
				if (tmpAngleEnd < 0)
					tmpAngleEnd += 360;
				if (((tmpAngle - tmpAngleEnd < 180)&&(tmpAngleEnd<tmpAngle)))
					filledPieRGBA(
						gf2d_graphics_get_renderer(),
						self->position.x, self->position.y,
						vector2d_magnitude(end),
						tmpAngleEnd,
						tmpAngle,
						150, 255, 10 + i * 5, 70
						);
				if ((tmpAngle - tmpAngleEnd < -180))
					filledPieRGBA(
					gf2d_graphics_get_renderer(),
					self->position.x, self->position.y,
					vector2d_magnitude(end),
					tmpAngleEnd,
					tmpAngle,
					150, 255, 10 + i * 5, 70
					);
			}
			else
			{
				filledTrigonRGBA(
					gf2d_graphics_get_renderer(),
					x[i - 1], y[i - 1],
					x[i], y[i],
					self->position.x, self->position.y,
					150, 255, 10 + i * 5, 70
					);
			}
		}
		if ((hit->other == NULL)||(hit->hitpoint.x == hit->other->corners[3].x)|| (hit->hitpoint.x == hit->other->corners[0].x))
			lastOther = false;
		else
		{

			// calculate the last direction
			if ((hit->hitpoint.x >= hit->other->corners[3].x)&&(hit->hitpoint.x <= hit->other->corners[0].x))
			{
				// we're either on the top or bottom side, point in the x direction
				if (self->position.x > hit->hitpoint.x)
				{
					if (self->position.y > hit->hitpoint.y)
						lastDirection = vector2d(-1,0);
					else
						lastDirection = vector2d(1,0);
				}
				else
				{
					if (self->position.y > hit->hitpoint.y)
						lastDirection = vector2d(-1,0);
					else
						lastDirection = vector2d(1,0);
				}
			}
			else
			{
				if (self->position.y > hit->hitpoint.y)
				{
					if (self->position.x > hit->hitpoint.x)
						lastDirection = vector2d(0, 1);
					else
						lastDirection = vector2d(0, -1);
				}
				else
				{
					if (self->position.x > hit->hitpoint.x)
						lastDirection = vector2d(0, 1);
					else
						lastDirection = vector2d(0, -1);
				}
			}
			lastOther = true;
		}
		raycasthit_free(hit);
		i++;
	}
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
	int i, j;
	Vector2D v1, v2, diff;

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
				vector2d_sub(diff, direction, start);
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
	entitiy_update_all_colliders();
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

void entity_system_close()
{
	slog("closing entity system..");
}