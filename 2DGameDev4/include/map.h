#include "tile.h"

#ifndef _MAP_H_
#define _MAP_H_

typedef struct Map_S
{
	int tile[64][64];
} Map;

typedef struct
{
	Uint32 width, height;
	Vector2D start, end;
	Sprite *tileset;
	char *map;
}TileMap;

void draw_map(Map *m);

TileMap *tilemap_load(char *filename);

void tilemap_draw(TileMap *tilemap, Vector2D position);

void tilemap_load_walls(TileMap *tilemap, Vector2D position);
#endif