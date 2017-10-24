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

void enemy_set_path(Entity *self)
{
	PF_Path *path = NULL;
	PF_PathArray *patharray = NULL;
	Vector2D start;
	TileMap *map;
	map = get_current_tilemap();
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
	if (path != NULL)
	{
		patharray = convert_path_to_vector2d_array(path);
		path_free_all_parents(path);
		if (patharray != NULL)
		{
			if (self->patharray != NULL)
			{
				patharray_free(self->patharray);
			}
			self->patharray = patharray;
			self->pathlocation = patharray->count - 1;
		}
	}
}


bool move_along_path(PF_PathArray *patharray, Entity *self, Vector2D start, TileMap *map, Vector2D forward)
{
	Vector2D goal, diff;
	int i;
	double remainingDist = self->moveSpeed;
	if (!self || !patharray || !map)
		return false;
	if (patharray->count >= 1)
	{
		// only do this until you're out of path
		for (i = self->pathlocation; i >= 0; i--)
		{
			if ((patharray->path[i].x == self->lastPosition.x) && (patharray->path[i].y == self->lastPosition.y))
			{
				if (patharray->count == 1)
				{
					self->currentDestination += 1;
					if ((self->currentDestination > self->numPatrol) && (self->alert == 0))
						self->currentDestination = 0;
					else if ((self->alert == 1) && (self->currentDestination > 3))
					{
						self->huntLoops += 1;
						self->currentDestination = 0;
					}
					enemy_set_path(self);
					return true;
				}
				continue;
			}
			goal = vector2d(map->position.x + (patharray->path[i].x * map->tileset->frame_w), map->position.y + (patharray->path[i].y * map->tileset->frame_h));
			if (goal.x == self->position.x && goal.y == self->position.y)
				slog("moving to current position");
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
				self->pathlocation = i;
				if (i == 0)
				{
					// end of path..
					self->currentDestination += 1;
					if ((self->currentDestination > self->numPatrol) && (self->alert == 0))
						self->currentDestination = 0;
					else if ((self->alert == 1) && (self->currentDestination > 3))
					{
						self->currentDestination = 0;
						self->huntLoops += 1;
					}
					enemy_set_path(self);
					return true;
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
	return false;
}
void enemy_set_hunting_points(Entity *self, Vector2D base, double rad, TileMap *map)
{
	RaycastHit *hit;
	int i;
	if (!self)
		return;
	
	self->hunt[4] = vector2d((int)((base.x - map->position.x) / map->tileset->frame_w), (int)((base.y - map->position.y) / map->tileset->frame_h));
	self->currentDestination = 4;
	self->huntLoops = 0;
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
			else if (hit->hitpoint.x == hit->other->corners[0].x)
				hit->hitpoint.x += 0.0001;
			if (hit->hitpoint.y == hit->other->corners[2].y)
				hit->hitpoint.y -= 0.0001;
			else if (hit->hitpoint.y == hit->other->corners[0].y)
				hit->hitpoint.y += 0.0001;
		}
		self->hunt[i] = vector2d((int)((hit->hitpoint.x - map->position.x) / map->tileset->frame_w), (int)((hit->hitpoint.y - map->position.y) / map->tileset->frame_h));
		raycasthit_free(hit);
	}
}


void enemy_set_alert(Entity *self, Vector2D eyePos, Vector2D forward, double huntingRad, TileMap *map)
{
	Vector2D playerEye;
	Vector2D playerDirection;
	Entity *seen = NULL;
	Entity *seenAlt = NULL;
	Entity *playerSeen = NULL;
	Vector2D diff, diff2;
	seen = entity_closest_in_sight_by_layer(self, 2, eyePos, forward);
	if (seen != NULL)
	{
		// if we can see a player...
		if (self->alert != 3)
			self->lastShot = self->timer;
		vector2d_add(playerEye, seen->position, seen->eyePos);
		playerDirection = vector2d_rotate(seen->forward, seen->fov * GF2D_PI / -360);
		if (entity_can_i_see_you(seen, self, playerEye, playerDirection) == 1)
		{
			self->turn_to_stone(self);
			return;
		}
		self->alert = 3;
		self->seeThePlayer = true;
		self->lastKnownPosition = seen->position;
		return;
	}
	else if (self->alert == 3)
	{
		slog("i can no longer see the player");
		self->alert = 1;
		enemy_set_hunting_points(self, self->lastKnownPosition, self->huntRadius, map);
		enemy_set_path(self);
		self->seeThePlayer = false;
		return;
	}
	seen = entity_closest_in_sight_by_layer(self, 6, eyePos, forward);
	if (seen != NULL)
	{
		// if we find a stone...
		self->alert = 2;
		self->wasRetreating = true;
		enemy_set_path(self);
		return;
	}
	seen = entity_closest_in_sight_by_layer(self, 5, eyePos, forward);
	seenAlt = entity_closest_in_sight_by_layer(self, 7, eyePos, forward);
	if ((seen != NULL)||(seenAlt != NULL))
	{
		// funnel seenAlt into seen if it's relevant, do calculations with whatever's in seen by the end of it
		if (!seen)
			seen = seenAlt;
		else if (seenAlt != NULL)
		{
			vector2d_sub(diff, seen->position, eyePos);
			vector2d_sub(diff2, seenAlt->position, eyePos);
			if (vector2d_magnitude_squared(diff2) < vector2d_magnitude_squared(diff))
				seen = seenAlt;
		}
		if (self->alert != 1)
		{
			enemy_set_hunting_points(self, seen->position, huntingRad, map);
			enemy_set_path(self);
		}
		// if we find a corpse...
		self->alert = 1;
		return;
	}
	if (self->alert == 1)
	{
		if (self->huntLoops > 2)
			self->alert = 0;
	}
}

void archer_draw(Entity *self)
{
	Vector2D direction, eyePos;
	Vector4D color;
	vector2d_add(eyePos, self->position, self->eyePos);
	direction = self->forward;
	direction = vector2d_rotate(direction, self->fov * GF2D_PI / -360);
	if (self->alert != 0)
	{
		color = vector4d(0, 0, 70, 255);
	}
	else
	{
		color = vector4d(0, 0, 160, 0);
	}
	vector2d_set_magnitude(&direction, self->maxSight);
	draw_line_of_sight(self, 1, self->fov, direction, color, 20, eyePos);
}

void archer_update(Entity *self)
{
	Vector2D direction;
	TileMap *map;
	Vector2D start, goal, diff;
	Vector4D color;
	PF_Path *path = NULL;
	PF_PathArray *patharray;
	Vector2D eyePos;
	Entity *projectile;
	vector2d_add(eyePos, self->position, self->eyePos);
	double remainingDist = self->moveSpeed;
	int i;
	if (self->timer == 0)
		enemy_set_path(self);
	self->timer += 1;
	map = get_current_tilemap();
	direction = self->forward;
	direction = vector2d_rotate(direction, self->fov * GF2D_PI / -360);
	enemy_set_alert(self, eyePos, direction, self->huntRadius, map);
	//self->forward = vector2d_rotate(self->forward, 0.01);
	vector2d_sub(start, self->position, map->position);
	start = vector2d((int)(start.x / map->tileset->frame_w), (int)(start.y / map->tileset->frame_h));
	if (self->alert == 3)
	{
		if ((self->projectile != NULL)&&(self->timer - self->lastShot > self->reload))
		{
			projectile = entity_new();
			if (projectile != NULL)
			{
				entity_copy_prefab(projectile, self->projectile);
				projectile->position = eyePos;
				projectile->parent = self;
				vector2d_sub(projectile->velocity, self->lastKnownPosition, projectile->position);
				vector2d_set_magnitude(&projectile->velocity, 10);
				self->lastShot = self->timer;
			}
		}
	}
	if (self->patharray == NULL)
		return;
	if (self->alert == 2)
	{
		if (move_along_path(self->patharray, self, start, map, direction) == true)
		{
			if (self->wasRetreating == true)
			{
				self->behindMe = vector2d(self->forward.x * -1, self->forward.y * -1);
				self->wasRetreating = false;
			}
			enemy_turn_toward_vector(self, self->behindMe);
		}
	}
	else
	{
		// if we can see the player.. don't bother moving, we're close enough
		if (self->alert == 3)
			return;
		move_along_path(self->patharray, self, start, map, direction);
	}
}

void archer_init(Entity *self)
{
	if (!self)
		return;
	if (self->coll != NULL)
	{
		self->eyePos = vector2d(
			self->coll->width / 2,
			self->coll->height / 2);
	}
	self->draw = archer_draw;
}

void archer_touch(Entity *self, Entity *other)
{

}

void archer_take_damage(Entity *self, int damage)
{
	if (!self)
		return;
	self->health -= damage;
	if (self->ouch != NULL)
		sound_play(self->ouch, 0, self->ouch->volume, self->ouch->defaultChannel, 0);
	if (self->health <= 0)
	{
		self->die(self);
	}
}

void archer_free(Entity *self)
{
	entity_free(self);
}

void archer_die(Entity *self)
{
	Entity *remains;
	if (self->corpse != NULL)
	{
		slog("archer is dying. replacing with corpse");
		remains = entity_new();
		entity_copy_prefab(remains, self->corpse);
		remains->position = self->position;
	}
	archer_free(self);
}

void archer_turn_to_stone(Entity *self)
{
	Entity *statue;
	if (self->stone != NULL)
	{
		slog("archer is turning to stone. replacing with stone");
		statue = entity_new();
		entity_copy_prefab(statue, self->stone);
		statue->position = self->position;
	}
	archer_free(self);
	get_current_tilemap()->numEnemies -= 1;
}

void stone_touch(Entity *self, Entity *other)
{
	if (!self || !other)
		return;
	if (other->layer == 1)
		self->position = self->lastPosition;
	if (self->layer == 5)
		slog("meat is touching D:");
}

void corpse_free(Entity *self)
{
	entity_free(self);
}

void corpse_die(Entity *self)
{
	Entity *bones;
	if (!self)
		return;
	if (self->corpse != NULL)
	{
		if (self->corpse->sprite != NULL)
		{
			self->sprite = self->corpse->sprite;
			self->spriteOffset = self->corpse->spriteOffset;
			self->scale = self->corpse->scale;
			self->frame = 0;
		}
		self->layer = self->corpse->layer;
		get_current_tilemap()->numEnemies -= 1;
	}
}