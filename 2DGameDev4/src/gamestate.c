#include "gamestate.h"
#include "simple_logger.h"
#include "input.h"
#include "map_editor.h"

Bool done = false;
Bool is_level = false;
Bool is_editing = false;

Bool game_done()
{
	return done;
}

void game_win()
{
	
}

void game_lose()
{

}

void game_set_done_true()
{
	done = true;
}

void game_set_editor_true()
{
	is_editing = true;
}

Bool game_is_editing()
{
	return is_editing;
}

void game_start_map_editor()
{
	game_load_map("editor/base_level.map");
	slog("Okay, we're loading the base map editor level!");
	game_set_editor_true();
	map_editor_init();
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