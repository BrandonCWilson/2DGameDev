#ifndef _TILE_H_
#define _TILE_H_

#include "gf2d_sprite.h"

typedef struct Tile_S
{
	char *name;
	int ID;
	bool inUse;
	Sprite *sprite;
	int frame;
	bool collides;
}Tile;

Tile *tile_new();

void tile_init(int max);

Tile * tile_get_by_id(int ID);

void tile_delete(Tile *tile);

void tile_system_close();

#endif // _TILE_H_
