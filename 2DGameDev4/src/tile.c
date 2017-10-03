#include "tile.h"
#include "simple_logger.h"

typedef struct 
{
	Uint32 max_tiles;
	Tile * tile_list;
} TileManager;

static TileManager tile_manager;

void tile_init(int max)
{
	int i;
	slog("initializing entity system..");
	if (!max)
	{
		slog("cannot initialize an entity system for zero entities!");
		return;
	}
	tile_manager.max_tiles = max;
	tile_manager.tile_list = (Tile *)malloc(sizeof(Tile)*max);
	memset(tile_manager.tile_list, 0, sizeof(Tile)*max);

	slog("entity system initialized");
	atexit(tile_system_close);
}

Tile *tile_new()
{
	int i;
	/*search for an unused entity address*/
	for (i = 0; i < tile_manager.max_tiles; i++)
	{
		if (!tile_manager.tile_list[i].inUse)
		{
			tile_delete(&tile_manager.tile_list[i]);	// clean up the old data
			tile_manager.tile_list[i].inUse = true;	// set it to inUse
			return &tile_manager.tile_list[i];			// return address of this array element
		}
	}
	slog("error: out of tile addresses");
	return NULL;
}

void tile_delete(Tile *tile)
{
	if (!tile)return;
	memset(tile, 0, sizeof(Tile));	//clean up the data
}

void tile_free(Tile *tile)
{
	if (!tile)return;
	tile->inUse = false;
}

Tile * tile_get_by_id(int ID)
{
	Tile *rtn = NULL;
	int i;
	for (i = 0; i < tile_manager.max_tiles; i++)
	{
		if (tile_manager.tile_list[i].ID == ID)
		{
			rtn = &tile_manager.tile_list[i];
			break;
		}
	}
	return rtn;
}

void tile_system_close()
{
	slog("Closing tile system..");
	// do something
}