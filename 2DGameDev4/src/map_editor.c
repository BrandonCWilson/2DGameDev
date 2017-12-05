#include "map_editor.h"
#include "entity.h"
#include "input.h"
#include "simple_logger.h"
#include <stdio.h>

FILE *mapfile = NULL;
int selected_tile = 0;

void map_editor_close()
{
	int i, j;
	if (mapfile != NULL)
	{
		fprintf(mapfile, "width: %i\n", get_current_tilemap()->width);
		fprintf(mapfile, "height: %i\n", get_current_tilemap()->height);
		fprintf(mapfile, "tileset: %s %i,%i,%i\n", get_current_tilemap()->tileset->filepath, get_current_tilemap()->tileset->frame_w, get_current_tilemap()->tileset->frame_h, get_current_tilemap()->tileset->frames_per_line);
		fprintf(mapfile, "map_begin\n");
		for (j = 0; j < get_current_tilemap()->height; j++)
		{
			for (i = 0; i < get_current_tilemap()->width; i++)
			{
				fprintf(mapfile, "%c", get_current_tilemap()->map[j * get_current_tilemap()->width + i]);
			}
			fprintf(mapfile, "\n");
		}
		fprintf(mapfile, "map_end\n");
		fclose(mapfile);
	}
	mapfile = NULL;
}

void cursor_update(Entity *self)
{
	TileMap *map;
	Vector2D start, curTile;

	self->timer += 1;
	if ((input_get_axis(INPUT_AXIS_FWD_X) > 10000)&&(self->timer - self->lastHit > 15))
	{
		tilemap_expand(get_current_tilemap(), 1, 0);
		self->lastHit = self->timer;
	}
	if ((input_get_axis(INPUT_AXIS_FWD_X) < -10000) && (self->timer - self->lastHit > 15))
	{
		tilemap_expand(get_current_tilemap(), -1, 0);
		self->lastHit = self->timer;
	}
	if ((input_get_axis(INPUT_AXIS_FWD_Y) > 10000) && (self->timer - self->lastHit > 15))
	{
		tilemap_expand(get_current_tilemap(), 0, 1);
		self->lastHit = self->timer;
	}
	if ((input_get_axis(INPUT_AXIS_FWD_Y) < -10000) && (self->timer - self->lastHit > 15))
	{
		tilemap_expand(get_current_tilemap(), 0, -1);
		self->lastHit = self->timer;
	}

	if ((input_get_button(INPUT_BUTTON_CHARGE)) && (self->timer - self->lastHit > 15))
	{
		selected_tile += 1;
		if (selected_tile > 3)
			selected_tile = 3;
		self->frame = selected_tile;
		self->lastHit = self->timer;
	}
	if ((input_get_button(INPUT_BUTTON_PULL)) && (self->timer - self->lastHit > 15))
	{
		selected_tile -= 1;
		if (selected_tile < 0)
			selected_tile = 0;
		self->frame = selected_tile;
		self->lastHit = self->timer;
	}

	self->velocity.x = self->moveSpeed * input_get_axis(INPUT_AXIS_MOVE_X) / (double)10000;
	self->velocity.y = self->moveSpeed * input_get_axis(INPUT_AXIS_MOVE_Y) / (double)10000;
	self->forward = vector2d(input_get_axis(INPUT_AXIS_FWD_X), input_get_axis(INPUT_AXIS_FWD_Y));
	if (vector2d_magnitude_squared(self->velocity) < 0.1)
	{
		self->velocity = vector2d(0, 0);
	}
	else
		self->lastForward = self->velocity;
	if (vector2d_magnitude_squared(self->forward) < 40000000)
	{
		self->forward = self->lastForward;
	}
	vector2d_set_magnitude(&self->forward, self->maxSight);

	if (input_get_button(INPUT_BUTTON_SELECT))
	{
		map = get_current_tilemap();
		if (!map)
		{
			slog("Unable to find a map..");
			return NULL;
		}

		vector2d_sub(start, self->position, map->position);
		curTile = vector2d((int)(start.x / map->tileset->frame_w), (int)(start.y / map->tileset->frame_h));
		if ((curTile.x / map->width >= 1)||(curTile.y / map->height >= 1))
			slog("You really shouldn't be touching this memory, my dude...");
		else
			map->map[(int)curTile.y * map->width + (int)curTile.x] = '0' + selected_tile;
	}
}

void cursor_init()
{
	Entity *cursor = NULL;
	cursor = entity_new();
	if (!cursor) return;
	cursor->update = cursor_update;
	cursor->sprite = gf2d_sprite_load_all("images/tileset.png", 32, 32, 4);
	slog("Frames per line: %i", cursor->sprite->frames_per_line);
	//cursor->sprite->frames_per_line = 4;
	cursor->scale = vector2d(1, 1);
	cursor->position = vector2d(26, 26);
	cursor->frame = 0;
	cursor->colorShift = vector4d(100, 100, 0, 100);
	cursor->singleFrame = true;
	cursor->moveSpeed = 0.85;
}

void map_editor_init()
{
	char *filetemplate = "levels/customlevel(%i).map";
	char filename[256];
	int i;
	cursor_init();
	mapfile = fopen("levels/customlevel.map", "r");
	if (mapfile == NULL)
	{
		// file does not exist, write to it.
		mapfile = fopen("levels/customlevel.map", "w");
		return;
	}
	fclose(mapfile);
	// find a valid filename for this new map
	for (i = 1; ; i++)
	{
		slog("Looking for a suitable map name.. %i", i);
		snprintf(filename, sizeof(filename), filetemplate, i);
		slog("Trying to read file.. %s", filename);
		mapfile = fopen(filename, "r");
		if (mapfile == NULL)
		{
			slog("Unable to open file: %s", filename);
			break;
		}
		else
		{
			slog("Filename is taken: %s", filename);
		}
		slog("Closing map: %s", filename);
		fclose(mapfile);
		slog("map is closed: %s", filename);
	}
	slog("Found a suitable map! Writing..");
	mapfile = fopen(filename, "w");
	if (!mapfile)
		slog("Failed to open a map file for writing");
}