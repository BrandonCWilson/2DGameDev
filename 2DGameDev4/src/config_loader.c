#include "config_loader.h"
#include <string.h>
#include "simple_logger.h"

int level_MAX_WIDTH = 0;
int level_MAX_HEIGHT = 0;

typedef struct 
{
	Entity *prefab_list;
	int max;
} PrefabManager;

PrefabManager prefab_manager;

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
	FILE *inFile;
	int numEnts;
	char param[255];
	char input[255];
	int i = 0;
	int j;
	// count the incoming prefabs and make some space
	numEnts = config_loader_entities_init_count_incoming(filename);
	
	prefab_manager.prefab_list = (Entity *)malloc(sizeof(Entity)*numEnts);
	memset(prefab_manager.prefab_list, 0, sizeof(Entity)*numEnts);
	prefab_manager.max = numEnts;

	inFile = fopen(filename, "r");
	if (!inFile)
	{
		slog("Unable to access the prefab file ents");
		return;
	}
	while (fscanf(inFile, "%s %s\n", param, input) != EOF)
	{
		if (strcmp(param, "ent:") == 0)
		{
			for (j = 0; j < 255; j++)
			{
				prefab_manager.prefab_list[i].name[j] = input[j];
			}
			while (strcmp(param, "END") != 0)
			{
				if (strcmp(param, "update:") == 0)
				{
					prefab_manager.prefab_list[i].update = config_loader_char_to_function(input);
				}
				fscanf(inFile, "%s %s", param, input);
			}
			i++;
		}
	}
	fclose(inFile);
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

}

void config_loader_levels(char *filename)
{
	// should use some linked list struct to define the order of levels
	// should use some list struct to add an unknown number of map configurations
	FILE *levelFile = fopen(filename, "r");
	char param[255];
	char input[255];

	slog("config loading levels");
	if (!levelFile)
	{
		perror(filename);
		return;
	}
	while (fscanf(levelFile, "%s %s", param, input) != EOF)
	{
		if (strcmp(param, "map:") == 0)
			config_loader_map_init(input);
	}
	
	fclose(levelFile);
}

FunctionParser funct[] =
{
	{ player_update, "player_update" },
	{ clickerUpdate, "clickerUpdate" }
};