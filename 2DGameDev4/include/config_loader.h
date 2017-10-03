#include <stdio.h>
#include "player.h"
#include "testUpdate.h"

#ifndef _CONFIG_LOADER_H_
#define _CONFIG_LOADER_H_

typedef struct
{
	void *function;
	char *name;
} FunctionParser;

void config_loader_entities_init(char *filename);

/*
* @brief read entity information and default values from config files
*/
void config_loader_entities();

/*
* @brief read maps and level information from config files
*/
void config_loader_levels(char *filename);

Entity * config_loader_get_prefab_by_name(char *name);

FunctionParser funct[];
#endif