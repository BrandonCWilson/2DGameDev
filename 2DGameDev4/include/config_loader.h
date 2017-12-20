#include <stdio.h>
#include "player.h"
#include "testUpdate.h"
#include "map.h"

#ifndef _CONFIG_LOADER_H_
#define _CONFIG_LOADER_H_

typedef struct
{
	void *function;
	char *name;
} FunctionParser;

typedef struct
{
	Entity *prefab_list;
	int max;
} PrefabManager;

void config_loader_entities_init(char *filename);

/*
* @brief read entity information and default values from config files
*/
void config_loader_entities();

/*
* @brief read maps and level information from config files
*/
void config_loader_levels(char *filename);

PrefabManager *config_loader_get_prefab_manager();

Entity * config_loader_get_prefab_by_name(char *name);

void level_list_init();

PriorityQueueList *level_list_get();

FunctionParser funct[];
#endif