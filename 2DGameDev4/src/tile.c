#include "tile.h"

void tileset_free(Tileset *tileset)
{
	if (!tileset)
	{
		slog("Cannot free a NULL pointer to a tileset");
		return NULL;
	}

	if (tileset->tiles != NULL)
		free(tileset->tiles);
	gf2d_sprite_free(tileset->sprite);
	free(tileset);
}

Tileset *tileset_new(Uint32 max, Sprite *sprite)
{
	Tileset *rtn;
	if (max < 0)
	{
		slog("Cannot allocate a tileset for 0 tiles!");
		return NULL;
	}

	rtn = (Tileset *)malloc(sizeof(Tileset));
	if (!rtn)
	{
		slog("Unable to allocate memory for the tileset");
		return NULL;
	}
	memset(rtn, 0, sizeof(Tileset));

	rtn->tiles = (Tile *)malloc(sizeof(Tile)*max);
	if (!rtn->tiles)
	{
		slog("Unable to allocate memory for the tile array in the tileset");
		return NULL;
	}
	memset(rtn->tiles, 0, sizeof(Tile)*max);
	rtn->max = max;

	rtn->sprite = sprite;

	return rtn;
}