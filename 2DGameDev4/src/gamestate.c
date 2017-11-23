#include "gamestate.h"
#include "simple_logger.h"
#include "input.h"

Bool done = false;
Bool is_level = false;

Bool game_done()
{
	return done;
}

void game_set_done_true()
{
	done = true;
}

void game_load_map(char *map)
{
	TileMap *newMap = NULL;
	if (!map) return;
	if (get_current_tilemap() != NULL) game_close_map();
	newMap = tilemap_load(map, vector2d(86, 24));
	if (!newMap) return;
	pathfinding_generate_graph_from_tilemap(newMap);
	tilemap_load_walls(newMap, vector2d(86, 24));
	is_level = true;
	// there was a bug with the input that I don't entirely understand
	// it loses track of the controller during map loading
	// this fixes it
	input_init();
}

void game_close_map()
{
	tilemap_free(get_current_tilemap());
	entity_clear_all();
	is_level = false;
}

void game_restart_map(TileMap *oldMap, char *newMap)
{
	if (!oldMap || !newMap) return;
	game_close_map(oldMap);
	game_load_map(newMap);
}

Bool game_is_level()
{
	return is_level;
}