#include "tile.h"

#ifndef _MAP_H_
#define _MAP_H_

typedef struct Map_S
{
	int tile[64][64];
} Map;

void draw_map(Map *m);
#endif