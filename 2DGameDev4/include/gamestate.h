#ifndef _GAMESTATE_H_
#define _GAMESTATE_H_

#include "gf2d_types.h"
#include "map.h"
#include "entity.h"

Bool game_done();

void game_load_map(char *map);

void game_set_done_true();

void game_restart_map(TileMap *oldMap, char *newMap);

void game_close_map();

Bool game_is_level();

Bool game_is_editing();

void game_start_map_editor();

void game_set_editor_true();

#endif