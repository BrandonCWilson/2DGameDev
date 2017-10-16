#include "enemy.h"
#include "simple_logger.h"
#include "gf2d_draw.h"
#include "pathfinding.h"

void archer_update(Entity *self)
{
	Vector2D direction;
	Vector4D color;
	PF_Path *path;
	PF_PathArray *patharray;
	if (entity_closest_in_sight_by_layer(self, 2) == get_player())
		color = vector4d(0, 255, 100, 120);
	else
		color = vector4d(0, 0, 100, 0);
	direction = self->forward;
	vector2d_set_magnitude(&direction, self->maxSight);
	draw_line_of_sight(self, 1, 90, direction, color, 20);
	self->forward = vector2d_rotate(self->forward, 0.05);
	slog("get path");
	path = pathfinding_get_path(get_current_graph(), vector2d(5, 5), vector2d(5, 5));
	slog("convert path");
	patharray = convert_path_to_vector2d_array(path);
	gf2d_draw_line(patharray->path[0], patharray->path[patharray->count - 1], vector4d(255, 255, 255, 255));

	path_free_all_parents(path);
	// FIXME bug? unable to execute both of these free functions
	patharray_free(patharray);
}

void archer_init(Entity *self)
{

}

void archer_touch(Entity *self, Entity *other)
{

}