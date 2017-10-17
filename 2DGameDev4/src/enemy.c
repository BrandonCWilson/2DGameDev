#include "enemy.h"
#include "simple_logger.h"
#include "gf2d_draw.h"
#include "pathfinding.h"
#include "map.h"

bool enemy_turn_toward_vector(Entity *self, Vector2D goal)
{
	Vector2D dir, end;
	double initialAngle, endAngle;
	if (vector2d_magnitude_squared(goal) == 0)
		return true;
	initialAngle = vector2d_angle(self->forward); endAngle = vector2d_angle(goal);
	if (initialAngle < 0)
		initialAngle = initialAngle + 360;
	if (endAngle < 0)
		endAngle = endAngle + 360;
	if (endAngle - initialAngle < 0)
	{
		endAngle = endAngle + 360;
	}
	// if we can make it
	if ((initialAngle - endAngle < self->turnSpeed)&&(initialAngle - endAngle > -1*self->turnSpeed))
	{
		self->forward = goal;
		return true;
	}
	// gotta turn again
	else
	{
		if (endAngle - initialAngle <= 180)
			self->forward = vector2d_rotate(self->forward, self->turnSpeed * GF2D_PI / 180);
		else
			self->forward = vector2d_rotate(self->forward, self->turnSpeed * GF2D_PI / -180);
		return false;
	}
}

void move_along_path(PF_PathArray *patharray, Entity *self, Vector2D start, TileMap *map, Vector2D forward)
{
	Vector2D goal, diff;
	int i;
	double remainingDist = self->moveSpeed;
	if (!self || !patharray || !map)
		return;
	if (patharray->count >= 1)
	{
		// only do this until you're out of path
		for (i = patharray->count - 1; i >= 0; i--)
		{
			if ((patharray->path[i].x == self->lastPosition.x) && (patharray->path[i].y == self->lastPosition.y))
			{
				if (patharray->count == 1)
				{
					self->currentDestination += 1;
					if ((self->currentDestination > self->numPatrol) && (self->alert == 0))
						self->currentDestination = 0;
					else if ((self->alert == 1) && (self->currentDestination > 3))
						self->currentDestination = 0;
				}
				continue;
			}
			goal = vector2d(map->position.x + (patharray->path[i].x * map->tileset->frame_w), map->position.y + (patharray->path[i].y * map->tileset->frame_h));
			if (goal.x == self->position.x && goal.y == self->position.y)
				slog("moving to current position");
			// move toward goal
			if (i < patharray->count - 2)
				break;
			vector2d_sub(diff, goal, self->position);
			// turn toward the point, and stand still if we have to turn again
			if (enemy_turn_toward_vector(self, diff) == false)
				break;
			// if we are going to move past the path point
			if (vector2d_magnitude(diff) < remainingDist)
			{
				// move to the point and try again
				remainingDist -= vector2d_magnitude(diff);
				self->position = goal;
				self->lastPosition = patharray->path[i];
				if (i == 0)
				{
					// end of path..
					self->currentDestination += 1;
					if ((self->currentDestination > self->numPatrol) && (self->alert == 0))
						self->currentDestination = 0;
					else if ((self->alert == 1) && (self->currentDestination > 3))
						self->currentDestination = 0;
				}
			}
			// we aren't gonna make it
			else
			{
				vector2d_set_magnitude(&diff, remainingDist);
				vector2d_add(self->position, self->position, diff);
				break;
			}
		}
	}
}
void enemy_set_hunting_points(Entity *self, Vector2D base, double rad, TileMap *map)
{
	RaycastHit *hit;
	int i;
	if (!self)
		return;
	slog("i wanna hunt");
	for (i = 0; i < 4; i++)
	{
		hit = raycast_through_all_entities(base, vector2d(base.x + rad * (i < 2 ? -1 : 1),base.y + rad * (i % 3 == 0 ? 1: -1)), 1);
		if (!hit)
		{
			slog("%i could not be raycasted");
			return;
		}
		if (hit->other != NULL)
		{
			// handle cases with the side of a wall
			if (hit->hitpoint.x == hit->other->corners[3].x)
				hit->hitpoint.x -= 0.0001;
			else
				hit->hitpoint.x += 0.0001;
		}
		self->hunt[i] = vector2d((int)((hit->hitpoint.x - map->position.x) / map->tileset->frame_w), (int)((hit->hitpoint.y - map->position.y) / map->tileset->frame_h));
		
		raycasthit_free(hit);
	}
}

void enemy_set_alert(Entity *self, Vector2D eyePos, Vector2D forward, double huntingRad, TileMap *map)
{
	Entity *seen = NULL;
	seen = entity_closest_in_sight_by_layer(self, 2, eyePos, forward);
	if (seen != NULL)
	{
		// if we can see the player...
		self->alert = 3;
		return;
	}
	seen = entity_closest_in_sight_by_layer(self, 5, eyePos, forward);
	if (seen != NULL)
	{
		// if we find a stone...
		self->alert = 2;
		return;
	}
	seen = entity_closest_in_sight_by_layer(self, 6, eyePos, forward);
	if (seen != NULL)
	{
		if (self->alert != 1)
		{
			enemy_set_hunting_points(self, seen->position, huntingRad, map);
		}
		// if we find a corpse...
		self->alert = 1;
		return;
	}
	self->alert = 0;
	return;
}

void archer_update(Entity *self)
{
	Vector2D direction;
	TileMap *map;
	Vector2D start, goal, diff;
	Vector4D color;
	PF_Path *path;
	PF_PathArray *patharray;
	Vector2D eyePos;
	vector2d_add(eyePos, self->position, self->eyePos);
	double remainingDist = self->moveSpeed;
	int i;
	self->timer += 1;
	map = get_current_tilemap();
	direction = self->forward;
	direction = vector2d_rotate(direction, self->fov * GF2D_PI / -360);
	if (self->alert != 1)
	{
		enemy_set_alert(self, eyePos, direction, self->huntRadius, map);
	}
	if (self->alert != 0)
	{
		color = vector4d(0, 0, 70, 255);
	}
	else
	{
		color = vector4d(255, 0, 70, 0);
	}
	
	vector2d_set_magnitude(&direction, self->maxSight);
	draw_line_of_sight(self, 1, self->fov, direction, color, 20, eyePos);
	//self->forward = vector2d_rotate(self->forward, 0.01);
	vector2d_sub(start, self->position, map->position);
	start = vector2d((int)(start.x / map->tileset->frame_w), (int)(start.y / map->tileset->frame_h));
	if (self->alert == 0)
	{
		path = pathfinding_get_path(get_current_graph(), start, self->patrol[self->currentDestination]);
	}
	else if (self->alert == 1)
	{
		path = pathfinding_get_path(get_current_graph(), start, self->hunt[self->currentDestination]);
	}
	else if (self->alert == 2)
	{
		path = pathfinding_get_path(get_current_graph(), start, self->retreat);
	}
	else
	{
		path = pathfinding_get_path(get_current_graph(), start, self->patrol[self->currentDestination]);
	}
	if (!path)
	{
		slog("no path to move along");
		self->currentDestination += 1;
		if (self->currentDestination > self->numPatrol)
			self->currentDestination = 0;
		return;
	}
	patharray = convert_path_to_vector2d_array(path);
	move_along_path(patharray, self, start, map, direction);
	path_free_all_parents(path);
	patharray_free(patharray);
}

void archer_init(Entity *self)
{
	if (self->coll != NULL)
	{
		self->eyePos = vector2d(
			self->coll->width / 2,
			self->coll->height / 2);
	}
}

void archer_touch(Entity *self, Entity *other)
{

}