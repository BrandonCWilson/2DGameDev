#include "enemy.h"
#include "simple_logger.h"
#include "gf2d_draw.h"
#include "pathfinding.h"
#include "map.h"

void move_along_path(PF_PathArray *patharray, Entity *self, Vector2D start, TileMap *map)
{
	Vector2D goal, diff;
	int i;
	double remainingDist = self->moveSpeed;
	if (patharray->count >= 1)
	{
		// only do this until you're out of path
		for (i = patharray->count - 1; i >= 0; i--)
		{
			if ((patharray->path[i].x == start.x) && (patharray->path[i].y == start.y))
			{
				continue;
			}
			goal = vector2d(map->position.x + (patharray->path[i].x * map->tileset->frame_w), map->position.y + (patharray->path[i].y * map->tileset->frame_h));
			// move toward goal
			if (i < patharray->count - 2)
				break;
			vector2d_sub(diff, goal, self->position);
			// if we are going to move past the path point
			if (vector2d_magnitude(diff) < remainingDist)
			{
				// move to the point and try again
				remainingDist -= vector2d_magnitude(diff);
				self->position = goal;
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

void archer_update(Entity *self)
{
	Vector2D direction;
	TileMap *map;
	Vector2D start, goal, diff;
	Vector4D color;
	PF_Path *path;
	PF_PathArray *patharray;
	double remainingDist = self->moveSpeed;
	int i;
	self->timer += 1;
	map = get_current_tilemap();
	if (entity_closest_in_sight_by_layer(self, 2) == get_player())
		color = vector4d(0, 255, 100, 120);
	else
		color = vector4d(0, 0, 100, 0);
	direction = self->forward;
	
	vector2d_set_magnitude(&direction, self->maxSight);
	draw_line_of_sight(self, 1, self->fov, direction, color, 20);
	self->forward = vector2d_rotate(self->forward, 0.05);
	vector2d_sub(start, self->position, map->position);
	start = vector2d((int)(start.x / map->tileset->frame_w), (int)(start.y / map->tileset->frame_h));
	path = pathfinding_get_path(get_current_graph(), start, vector2d(20, 10));
	if (!path)
	{
		slog("no path to move along");
		return;
	}
	patharray = convert_path_to_vector2d_array(path);
	move_along_path(patharray, self, start, map);
	path_free_all_parents(path);
	patharray_free(patharray);
}

void archer_init(Entity *self)
{

}

void archer_touch(Entity *self, Entity *other)
{

}