#include "map_editor.h"
#include "entity.h"
#include "input.h"
#include "simple_logger.h"
#include <stdio.h>
#include "config_loader.h"
#include "physfs.h"

PHYSFS_File *mapfile;
//FILE *mapfile = NULL;
int selected_tile = 0;
bool placingTiles = true;
bool placingPatrol = false;
int patrolCtr = 0;
int prefab_id = 0;
Entity *selected_prefab;
Entity *lastSpawn;

char *ent_descr;

void map_editor_close()
{
	int i, j;
	char tmp[1024];
	char *map_descr;
	slog("Closing map editor");

	map_descr = (char *)malloc(sizeof(char) * 4096);
	if (!map_descr)
	{
		slog("Unable to allocate data for the map description");
		return NULL;
	}
	memset(map_descr, 0, sizeof(char) * 4096);
	if (mapfile != NULL)
	{
		slog("Generating map description");
		sprintf(tmp, "width: %i\n", get_current_tilemap()->width);
		map_descr = strcat(map_descr, tmp);
		sprintf(tmp, "height: %i\n", get_current_tilemap()->height);
		map_descr = strcat(map_descr, tmp);
		sprintf(tmp, "tileset: %s %i,%i,%i\n", get_current_tilemap()->tileset->filepath, get_current_tilemap()->tileset->frame_w, get_current_tilemap()->tileset->frame_h, get_current_tilemap()->tileset->frames_per_line);
		map_descr = strcat(map_descr, tmp);
		sprintf(tmp, "map_begin\n");
		map_descr = strcat(map_descr, tmp);
		for (j = 0; j < get_current_tilemap()->height; j++)
		{
			for (i = 0; i < get_current_tilemap()->width; i++)
			{
				sprintf(tmp, "%c", get_current_tilemap()->map[j * get_current_tilemap()->width + i]);
				map_descr = strcat(map_descr, tmp);
			}
			sprintf(tmp, "\n");
			map_descr = strcat(map_descr, tmp);
		}
		sprintf(tmp, "map_end\n");
		map_descr = strcat(map_descr, tmp);
		slog("Map description is finished");

		slog("Output: \n%s%s", map_descr, ent_descr);

		PHYSFS_write(mapfile, map_descr, sizeof(char), strlen(map_descr));

		sprintf(tmp, "%s\n", ent_descr);

		PHYSFS_write(mapfile, ent_descr, sizeof(char), strlen(ent_descr));

		PHYSFS_close(mapfile);
	}
	mapfile = NULL;
	if (ent_descr != NULL)
		free(ent_descr);
	free(map_descr);
	ent_descr = NULL;
}

void save_ent(Entity *ent)
{
	char tmp[1024];
	TileMap *map;
	Vector2D start, curTile;
	int i;
	slog("Saving an ent to the ent list! %s", ent->name);
	if (!tmp)
	{
		slog("Unable to allocate space for a tmp char array");
		return NULL;
	}
	memset(tmp, 0, sizeof(char) * 1024);
	sprintf(tmp, "ent: %s\n", ent->name);
	if (!ent_descr)
	{
		slog("Ent description is not allcoated!");
		return;
	}
	ent_descr = strcat(ent_descr, tmp);
	slog("ent_descr %s", ent_descr);

	map = get_current_tilemap();
	if (!map)
	{
		slog("Unable to find a map..");
		return NULL;
	}

	vector2d_sub(start, ent->position, map->position);
	curTile = vector2d((int)(start.x / map->tileset->frame_w), (int)(start.y / map->tileset->frame_h));
	if ((curTile.x / map->width >= 1) || (curTile.y / map->height >= 1))
		slog("You really shouldn't be touching this memory, my dude...");
	else
	{
		sprintf(tmp, "position: %i,%i\n", (int)curTile.x, (int)curTile.y);
		ent_descr = strcat(ent_descr, tmp);
	}

	for (i = 0; i < 4; i++)
	{
		sprintf(tmp, "patrol: %i,%i\n", (int)ent->patrol[i].x, (int)ent->patrol[i].y);
		ent_descr = strcat(ent_descr, tmp);
	}
	ent_descr = strcat(ent_descr, "patrol_end\n");

	slog("Ent description: \n%s", ent_descr);
}

void cursor_update(Entity *self)
{
	TileMap *map;
	Vector2D start, curTile;
	PrefabManager *prefab_manager;
	Entity *spawn;

	self->timer += 1;
	if ((input_get_axis(INPUT_AXIS_FWD_X) > 10000) && (self->timer - self->lastHit > 15))
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

	if ((input_get_button(INPUT_BUTTON_MELEE))&&(self->timer - self->lastHit > 45)&&(!placingPatrol))
	{
		placingTiles = !placingTiles;
		if (!placingTiles)
		{
			slog("I want to place some prefabs!");
			prefab_id = 0;
			selected_prefab = &config_loader_get_prefab_manager()->prefab_list[prefab_id];
			self->sprite = selected_prefab->sprite;
			self->spriteOffset = selected_prefab->spriteOffset;
			self->frame = selected_prefab->frame;
		}
		else
			slog("I want to place some tiles!");
		self->lastHit = self->timer;
	}

	if (placingPatrol)
	{
		if (input_get_button(INPUT_BUTTON_SELECT) && self->timer - self->lastHit > 20)
		{
			self->lastHit = self->timer;
			map = get_current_tilemap();
			vector2d_sub(start, self->position, map->position);
			curTile = vector2d((int)(start.x / map->tileset->frame_w), (int)(start.y / map->tileset->frame_h));
			if ((curTile.x / map->width >= 1) || (curTile.y / map->height >= 1))
				slog("You really shouldn't be touching this memory, my dude...");
			else
				lastSpawn->patrol[patrolCtr] = curTile;

			patrolCtr += 1;
			if (patrolCtr >= 4)
			{
				save_ent(lastSpawn);
				placingPatrol = false;
			}
		}
		if (input_get_button(INPUT_HURT_SELF))
		{
			save_ent(lastSpawn);
			placingPatrol = false;
		}
	}
	else if (placingTiles)
	{
		if ((input_get_button(INPUT_BUTTON_CHARGE)) && (self->timer - self->lastHit > 15))
		{
			selected_tile += 1;
			if (selected_tile > 3)
				selected_tile = 3;
			self->frame = selected_tile;
		}
		if ((input_get_button(INPUT_BUTTON_PULL)) && (self->timer - self->lastHit > 15))
		{
			selected_tile -= 1;
			if (selected_tile < 0)
				selected_tile = 0;
			self->frame = selected_tile;
			self->lastHit = self->timer;
		}
	}
	else
	{
		if ((input_get_button(INPUT_BUTTON_CHARGE)) && (self->timer - self->lastHit > 15))
		{
			prefab_manager = config_loader_get_prefab_manager();
			if (prefab_id + 1 < prefab_manager->max)
			{
				selected_prefab = &prefab_manager->prefab_list[prefab_id + 1];
				prefab_id += 1;
			}
			self->sprite = selected_prefab->sprite;
			self->frame = selected_prefab->frame;
			self->lastHit = self->timer;
		}
		if ((input_get_button(INPUT_BUTTON_PULL)) && (self->timer - self->lastHit > 15))
		{
			prefab_manager = config_loader_get_prefab_manager();
			if (prefab_id - 1 >= 0)
			{
				selected_prefab = &prefab_manager->prefab_list[prefab_id - 1];
				prefab_id -= 1;
			}
			self->sprite = selected_prefab->sprite;
			self->spriteOffset = selected_prefab->spriteOffset;
			self->frame = selected_prefab->frame;
			self->lastHit = self->timer;
		}

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
		if (placingTiles)
		{
			map = get_current_tilemap();
			if (!map)
			{
				slog("Unable to find a map..");
				return NULL;
			}

			vector2d_sub(start, self->position, map->position);
			curTile = vector2d((int)(start.x / map->tileset->frame_w), (int)(start.y / map->tileset->frame_h));
			if ((curTile.x / map->width >= 1) || (curTile.y / map->height >= 1))
				slog("You really shouldn't be touching this memory, my dude...");
			else
				map->map[(int)curTile.y * map->width + (int)curTile.x] = '0' + selected_tile;
		}
		else if (!placingPatrol)
		{
			// place a prefab
			spawn = entity_new();
			if (!spawn)
			{
				slog("Unable to spawn the prefab you wanted!");
				return NULL;
			}
			entity_copy_prefab(spawn, selected_prefab);
			spawn->position = self->position;
			spawn->update = NULL;
			strcpy(spawn->name, selected_prefab->name);
			lastSpawn = spawn;
			patrolCtr = 0;
			if (selected_prefab->name != NULL)
			{
				slog("I made a prefab! %s", selected_prefab->name);
			}
			else
				slog("Oh.. your prefab can't be real");
			if (selected_prefab->layer == 3)
				placingPatrol = true;
		}
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
	PHYSFS_file *physf = NULL;
	char *fileContents;
	int fileLength;

	
	slog("Starting map editor");
	cursor_init();
	if (PHYSFS_exists("levels/customlevel.map") == 0)
	{
		mapfile = PHYSFS_openAppend("levels/customlevel.map");
		return;
	}
	// find a valid filename for this new map
	for (i = 1; ; i++)
	{
		slog("Looking for a suitable map name.. %i", i);
		snprintf(filename, sizeof(filename), filetemplate, i);
		slog("Trying to read file.. %s", filename);
		if (PHYSFS_exists(filename) == 0)
		{
			mapfile = PHYSFS_openAppend(filename);
			if (!mapfile)
				slog("Error: %s", PHYSFS_getLastError());
			break;
		}
	}
	slog("Writing to file location: %s", filename);
	ent_descr = (char *)malloc(sizeof(char) * 32768);
	if (!ent_descr)
	{
		slog("Unable to allocate space for the entity descriptions in map making");
		return;
	}
	memset(ent_descr, 0, sizeof(char) * 32768);
	slog("Ent descr is properly allocated!");
}