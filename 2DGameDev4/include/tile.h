#ifndef _TILE_H_
#define _TILE_H_

#include "gf2d_sprite.h"

typedef struct Tile_s
{
	int frame;
	bool collides;
} Tile;

typedef struct Tileset_s
{
	Tile *tiles;
	int max;

	Sprite *sprite;
} Tileset;

Tileset *tileset_new(Uint32 max, Sprite *sprite);

#endif // _TILE_H_
