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
	,{ player_die, "player_die" }
	,{ player_take_damage, "player_take_damage"	}
	,{ archer_die, "archer_die" }
	,{ archer_take_damage, "archer_take_damage" }
	,{ arrow_die, "arrow_die" }
	,{ archer_turn_to_stone, "archer_turn_to_stone" }
	,{ stone_touch, "stone_touch" }
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

Entity * config_loader_get_prefab_by_name(char *name)
{
	int i;
	slog("max prefabs: %i", prefab_manager.max);
	for (i = 0; i < prefab_manager.max; i++)
	{
		slog("comparing.. %s %s", name, prefab_manager.prefab_list[i].name);
		if (strcmp(prefab_manager.prefab_list[i].name, name) == 0)
		{
			return &prefab_manager.prefab_list[i];
		}
	}
	slog("could not find the prefab you were looking for");
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
	slog("%i", rtn);
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
	numEnts = config_loader_entities_init_count_incoming(filename) + 1;
	
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
			slog("new prefab: %s", prefab_manager.prefab_list[i].name);
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
					slog("layer: %i", int1);
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
				if (strcmp(buffer, "turnspeed:") == 0)
				{
					fscanf(file, "%f", &scalex);
					prefab_manager.prefab_list[i].turnSpeed = scalex;
				}
				if (strcmp(buffer, "projectile:") == 0)
				{
					fscanf(file, "%s", buffer);
					prefab_manager.prefab_list[i].projectile = config_loader_get_prefab_by_name(buffer);
				}
				if (strcmp(buffer, "huntradius:") == 0)
				{
					fscanf(file, "%f", &scalex);
					prefab_manager.prefab_list[i].huntRadius = scalex;
				}
				if (strcmp(buffer, "die:") == 0)
				{
					fscanf(file, "%s", buffer);
					prefab_manager.prefab_list[i].die = config_loader_char_to_function(buffer);
				}
				if (strcmp(buffer, "takedamage:") == 0)
				{
					fscanf(file, "%s", buffer);
					prefab_manager.prefab_list[i].take_damage = config_loader_char_to_function(buffer);
				}
				if (strcmp(buffer, "health:") == 0)
				{
					fscanf(file, "%i", &int1);
					prefab_manager.prefab_list[i].health = int1;
				}
				if (strcmp(buffer, "corpse:") == 0)
				{
					fscanf(file, "%s", buffer);
					prefab_manager.prefab_list[i].corpse = config_loader_get_prefab_by_name(buffer);
				}
				if (strcmp(buffer, "reload:") == 0)
				{
					fscanf(file, "%i", &int1);
					prefab_manager.prefab_list[i].reload = int1;
				}
				if (strcmp(buffer, "stone:") == 0)
				{
					fscanf(file, "%s", buffer);
					prefab_manager.prefab_list[i].stone = config_loader_get_prefab_by_name(buffer);
				}
				if (strcmp(buffer, "turntostone:") == 0)
				{
					fscanf(file, "%s", buffer);
					prefab_manager.prefab_list[i].turn_to_stone = config_loader_char_to_function(buffer);
				}
				if (strcmp(buffer, "damage:") == 0)
				{
					fscanf(file, "%i", &int1);
					prefab_manager.prefab_list[i].damage = int1;
				}
				if (strcmp(buffer, "maxcharge:") == 0)
				{
					fscanf(file, "%i", &int1);
					prefab_manager.prefab_list[i].maxCharge = int1;
				}
				if (strcmp(buffer, "shotspeed:") == 0)
				{
					fscanf(file, "%f", &scalex);
					prefab_manager.prefab_list[i].shotSpeed = scalex;
				}
				fscanf(file, "%s", buffer);
				slog("%s", buffer);
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