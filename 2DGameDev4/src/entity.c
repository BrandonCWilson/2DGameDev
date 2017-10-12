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

void draw_line_of_sight(Entity *self, int layer, double fov)
{
	int i, j, numPoints;
	Sint16 *x, *y;
	Vector2D dir;
	Vector2D rotated;
	Vector2D end;
	Vector2D hitdiff;
	float initialAngle, endAngle;
	Vector2D initialHitpoint, endHitpoint;
	RaycastHit *hit = NULL;
	PriorityQueueList *pts;
	if (!self) return;
	pts = pqlist_new();
	if (!pts) return;
	// add the start and end of the fov
	dir = vector2d(-200, 0);
	rotated = vector2d_rotate(dir, fov * GF2D_PI / 180);
	vector2d_add(end, self->position, dir);
	hit = raycast_through_all_entities(self->position, end, layer);
	if (!hit) return;
	vector2d_sub(hitdiff, hit->hitpoint, self->position);
	gf2d_draw_line(self->position, hit->hitpoint, vector4d(255, 0, 0, 255));
	initialAngle = vector2d_angle(dir); initialHitpoint = hit->hitpoint;
	gf2d_draw_circle(initialHitpoint, 20, vector4d(0, 25, 25, 180));
	pqlist_insert(pts, hit, vector2d_angle(hitdiff));
	vector2d_add(end, self->position, rotated);
	hit = raycast_through_all_entities(self->position, end, layer);
	if (!hit) return;
	vector2d_sub(hitdiff, hit->hitpoint, self->position);
	gf2d_draw_line(self->position, hit->hitpoint, vector4d(255, 0, 0, 255));
	endAngle = vector2d_angle(rotated); endHitpoint = hit->hitpoint;
	gf2d_draw_circle(endHitpoint, 20, vector4d(0, 255, 25, 180));
	pqlist_insert(pts, hit, vector2d_angle(hitdiff));
	for (i = 0; i < entity_manager.max_entities; i++)
	{
		if (!entity_manager.ent_list[i].inUse) continue;
		if (!entity_manager.ent_list[i].coll) continue;
		if (entity_manager.ent_list[i].layer != layer) continue;
		for (j = 0; j < 4; j++)
		{
			vector2d_sub(dir, entity_manager.ent_list[i].coll->corners[j], self->position);
			end = entity_manager.ent_list[i].coll->corners[j];
			if ((vector2d_angle(dir) <= initialAngle)||(vector2d_angle(dir) >= endAngle)) continue;
			vector2d_set_magnitude(&dir, 200);
			vector2d_add(end, dir, self->position);
			hit = raycast_through_all_entities(self->position, end, layer);
			if (!hit)
				continue;
			vector2d_sub(hitdiff, hit->hitpoint, self->position);
			gf2d_draw_line(self->position, hit->hitpoint, vector4d(0, 255, 255, 255));
			pqlist_insert(pts, hit, vector2d_angle(hitdiff));
			if ((hit->hitpoint.x == entity_manager.ent_list[i].coll->corners[j].x)
				&& (hit->hitpoint.y == entity_manager.ent_list[i].coll->corners[j].y))
			{
				end = entity_manager.ent_list[i].coll->corners[j];
				vector2d_set_magnitude(&dir, 200);
				dir = vector2d_rotate(dir, 0.01);
				vector2d_add(end, dir, self->position);
				hit = raycast_through_all_entities(self->position, end, layer);
				if (!hit) continue;
				vector2d_sub(hitdiff, hit->hitpoint, self->position);
				pqlist_insert(pts, hit, vector2d_angle(hitdiff)); 
				end = entity_manager.ent_list[i].coll->corners[j];
				vector2d_set_magnitude(&dir, 200);
				dir = vector2d_rotate(dir, -0.02);
				vector2d_add(end, dir, self->position);
				hit = raycast_through_all_entities(self->position, end, layer);
				if (!hit) continue;
				vector2d_sub(hitdiff, hit->hitpoint, self->position);
				pqlist_insert(pts, hit, vector2d_angle(hitdiff));
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
		if (i > 0)
		{
			vector2d_sub(dir, hit->hitpoint, self->position);
			if (hit->other == NULL)
			{
				vector2d_sub(dir, vector2d(x[i - 1], y[i - 1]), self->position);
				vector2d_sub(end, vector2d(x[i], y[i]), self->position);
				if ((vector2d_angle(end) - vector2d_angle(dir) < fov) && ((vector2d_angle(end) - vector2d_angle(dir) > -fov)))
				{
					filledPieRGBA(
						gf2d_graphics_get_renderer(),
						self->position.x, self->position.y,
						vector2d_magnitude(end),
						vector2d_angle(end),
						vector2d_angle(dir),
						10, 255, 10, 100
						);
				}
			}
			else
			{
				// idk how this bug starts, but this stops it
				if (!(((double)x[i - 1] == initialHitpoint.x) && ((double)y[i - 1] == initialHitpoint.y)))
					filledTrigonRGBA(
					gf2d_graphics_get_renderer(),
					x[i - 1], y[i - 1],
					x[i], y[i],
					self->position.x, self->position.y,
					10, 255, 10, 100
					);
			}

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