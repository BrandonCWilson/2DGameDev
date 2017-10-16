#include "config_loader.h"
#include <string.h>
#include "simple_logger.h"
#include "priority_queue.h"
#include "testUpdate.h"
#include "projectile.h"
#include "enemy.h"

FunctionParser funct[] =
{
	{ player_update, "player_update" }
	,{ clickerUpdate, "clickerUpdate" }
	,{ player_init, "player_init" }
	,{ player_touch, "player_touch" }
	,{ arrow_update, "arrow_update" }
	,{ arrow_init, "arrow_init" }
	,{ arrow_touch, "arrow_touch" }
	,{ archer_init, "archer_init" }
	,{ archer_touch, "archer_touch" }
	,{ archer_update, "archer_update" }
};

int level_MAX_WIDTH = 0;
int level_MAX_HEIGHT = 0;

typedef struct 
{
	Entity *prefab_list;
	int max;
} PrefabManager;

PrefabManager prefab_manager;

PriorityQueue *map_list;

void * config_loader_char_to_function(char *fname)
{
	int i;
	for (i = 0; i < (sizeof(funct) / sizeof(funct[0])); i++)
	{
		if (strcmp(funct[i].name, fname) == 0)
		{
			return funct[i].function;
		}
	}
	return NULL;
}

Entity *copy_prefab(Entity *ent, Entity *prefab)
{
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
	ent->scale = prefab->scale;
	ent->spriteOffset = prefab->spriteOffset;
	ent->fov = prefab->fov;
	ent->maxSight = prefab->maxSight;
	ent->forward = vector2d(1, 0);
	ent->moveSpeed = prefab->moveSpeed;
}

Entity * config_loader_get_prefab_by_name(char *name)
{
	int i;
	for (i = 0; i < prefab_manager.max; i++)
	{
		if (strcmp(prefab_manager.prefab_list[i].name, name) == 0)
		{
			return &prefab_manager.prefab_list[i];
		}
	}
	return NULL;
}

int config_loader_entities_init_count_incoming(char *filename)
{
	FILE *f = fopen(filename, "r");
	char param[255];
	char input[255];
	int rtn = 0;

	if (!f)
	{
		slog("Unable to access the prefab file ents");
		return;
	}
	while (fscanf(f, "%s %s", param, input) != EOF)
	{
		if (strcmp(param, "ent:") == 0)
			rtn += 1;
	}
	fclose(f);
	return rtn;
}

void config_loader_entities_init(char *filename)
{
	FILE *file;
	char otherfile[1024];
	int int1, int2, int3, int4;
	double collheight, collwidth;
	float volume;
	float scalex, scaley;
	int numEnts;
	char buffer[1024];
	int i = 0;
	int j;
	// count the incoming prefabs and make some space
	numEnts = config_loader_entities_init_count_incoming(filename);
	
	prefab_manager.prefab_list = (Entity *)malloc(sizeof(Entity)*numEnts);
	memset(prefab_manager.prefab_list, 0, sizeof(Entity)*numEnts);
	prefab_manager.max = numEnts;

	file = fopen(filename, "r");
	if (!file)
	{
		slog("Unable to access the prefab file ents");
		return;
	}
	while (fscanf(file, "%s", buffer) != EOF)
	{
		if (strcmp(buffer, "ent:") == 0)
		{
			fscanf(file, "%s", prefab_manager.prefab_list[i].name);
			strcpy(buffer, prefab_manager.prefab_list[i].name);
			while (strcmp(buffer, "END") != 0)
			{
				if (strcmp(buffer, "update:") == 0)
				{
					fscanf(file, "%s", buffer);
					prefab_manager.prefab_list[i].update = config_loader_char_to_function(buffer);
				}
				if (strcmp(buffer, "sprite:") == 0)
				{
					fscanf(file, "%s %i,%i,%i", otherfile, &int1, &int2, &int3);
					if (strlen(otherfile) > 0)
						prefab_manager.prefab_list[i].sprite = gf2d_sprite_load_all(otherfile, int1, int2, int3);
				}
				if (strcmp(buffer, "boxcollider:") == 0)
				{
					fscanf(file, "%lf,%lf", &collwidth, &collheight);
					prefab_manager.prefab_list[i].coll = box_collider_new();
					prefab_manager.prefab_list[i].coll->width = collwidth;
					prefab_manager.prefab_list[i].coll->height = collheight;
				}
				if (strcmp(buffer, "ouch:") == 0)
				{
					fscanf(file, "%s %f,%i", otherfile, &volume, &int1);
					if (strlen(otherfile) > 0)
						prefab_manager.prefab_list[i].ouch = sound_load(otherfile, volume, int1);
				}
				if (strcmp(buffer, "layer:") == 0)
				{
					fscanf(file, "%i", &int1);
					prefab_manager.prefab_list[i].layer = int1;
				}
				if (strcmp(buffer, "init:") == 0)
				{
					fscanf(file, "%s", buffer);
					prefab_manager.prefab_list[i].init = config_loader_char_to_function(buffer);
				}
				if (strcmp(buffer, "touch:") == 0)
				{

					fscanf(file, "%s", buffer);
					prefab_manager.prefab_list[i].touch = config_loader_char_to_function(buffer);
				}
				if (strcmp(buffer, "scale:") == 0)
				{
					fscanf(file, "%f,%f", &scalex, &scaley);
					prefab_manager.prefab_list[i].scale = vector2d(scalex, scaley);
				}
				if (strcmp(buffer, "spriteoffset:") == 0)
				{
					fscanf(file, "%f,%f", &scalex, &scaley);
					prefab_manager.prefab_list[i].spriteOffset = vector2d(scalex, scaley);
				}
				if (strcmp(buffer, "fov:") == 0)
				{
					fscanf(file, "%i", &int1);
					prefab_manager.prefab_list[i].fov = int1;
				}
				if (strcmp(buffer, "maxsight:") == 0)
				{
					fscanf(file, "%f", &scalex);
					prefab_manager.prefab_list[i].maxSight = scalex;
				}
				if (strcmp(buffer, "movespeed:") == 0)
				{
					fscanf(file, "%f", &scalex);
					prefab_manager.prefab_list[i].moveSpeed = scalex;
				}
				fscanf(file, "%s", buffer);
			}
			i++;
			slog("%s", buffer);
		}
	}
	fclose(file);
	slog("Initialized the prefab array");
}

void config_loader_entities()
{
	// should use some list struct to add an unknown number of entity configurations
}

void config_loader_map_init(char *filename)
{
	// allocate memory based on information at the top of the file
	// such as max level width and height
	FILE *file;
	TileMap *map;
	char reader[1024];
	int framew, frameh, framesperline;
	if (!filename)
	{
		slog("no filename to load a map from...");
		return NULL;
	}
	file = fopen(filename, "r");
	if (!file)
	{
		slog("failed to open map file");
		return NULL;
	}
	map = (TileMap *)malloc(sizeof(TileMap));
	if (!map)
	{
		slog("failed to allocate map");
		return NULL;
	}
	memset(map, 0, sizeof(TileMap));
	while (fscanf(file, "%s", reader) != EOF)
	{
		if (strcmp(reader, "width:") == 0)
		{
			fscanf(file, "%ui", &map->width);
			continue;
		}
		if (strcmp(reader, "height:") == 0)
		{
			fscanf(file, "%ui", &map->height);
			continue;
		}
		if (strcmp(reader, "start:") == 0)
		{
			fscanf(file, "%lf, %lf", &map->start.x, &map->start.y);
			continue;
		}
		if (strcmp(reader, "end:") == 0)
		{
			fscanf(file, "%lf, %lf", &map->end.x, &map->end.y);
			continue;
		}
		if (strcmp(reader, "tileset:") == 0)
		{
			fscanf(file, "%s %i,%i,%i", reader, &framew, &frameh, &framesperline);
			if (strlen(reader) > 0)
			{
				map->tileset = gf2d_sprite_load_all(reader, framew, frameh, framesperline);
			}
			continue;
		}
		if (strcmp(reader, "map_begin") == 0)
		{
			if ((map->width * map->height) == 0)
			{
				slog("width and height need be defined before starting the tile map");
				tilemap_free(map);
				fclose(file);
				return NULL;
			}
			map->map = (char *)malloc(sizeof(char) * map->width * map->height);
			if (!map->map)
			{
				slog("failed to allocate data for map tile data");
				tilemap_free(map);
				fclose(file);
				return NULL;
			}
			memset(map->map, 0, sizeof(char) * map->width * map->height);
			while (fscanf(file, "%s", reader) != EOF)
			{
				if (reader[0] == '#')
				{
					continue;
				}
				if (strcmp("map_end", reader) == 0)
				{
					break;
				}
				strcat(map->map, reader);
			}
			continue;
		}
	}
	fclose(file);
	pqlist_insert(map_list, map, 1);
	return map;
}

int map_list_init()
{
	map_list = pqlist_new();
	if (!map_list)
	{
		slog("unable to allocate map_list data");
		return 0;
	}
	return 1;
}

void config_loader_levels(char *filename)
{
	// should use some linked list struct to define the order of levels
	// should use some list struct to add an unknown number of map configurations
	FILE *levelFile = fopen(filename, "r");
	char param[1024];
	char input[1024];

	slog("config loading levels");
	if (!levelFile)
	{
		perror(filename);
		return;
	}
	if (map_list_init() == 0)
		return;
	while (fscanf(levelFile, "%s %s", param, input) != EOF)
	{
		if (strcmp(param, "map:") == 0)
		{
			config_loader_map_init(input);
		}
	}
	
	fclose(levelFile);
}