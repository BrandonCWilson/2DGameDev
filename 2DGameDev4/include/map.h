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
	Vector2D position;
	int numEnemies;
}TileMap;

void draw_map(Map *m);

TileMap *tilemap_load(char *filename, Vector2D position);

void tilemap_draw_walkable(TileMap *tilemap, Vector2D position);

void tilemap_draw_walls(TileMap *tilemap, Vector2D position);

void tilemap_load_walls(TileMap *tilemap, Vector2D position);

TileMap *get_current_tilemap();

void tilemap_free(TileMap *tilemap);
#endif