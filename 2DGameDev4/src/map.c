#include "map.h"
#include "tile.h"
#include "gf2d_sprite.h"
#include "simple_logger.h"
#include "entity.h"
#include <stdio.h>
#include "player.h"
#include "config_loader.h"

TileMap *currentmap;

void set_current_tilemap(TileMap *map)
{
	currentmap = map;
}

TileMap *tilemap_new()
{
	TileMap *tilemap = NULL;
	tilemap = (TileMap *)malloc(sizeof(TileMap));
	if (!tilemap)
	{
		slog("failed to allocate tile map data");
		return NULL;
	}
	memset(tilemap, 0, sizeof(TileMap));
	return tilemap;
}

TileMap *get_current_tilemap()
{
	return currentmap;
}

void tilemap_shrink(TileMap *tilemap)
{

}

void tilemap_expand(TileMap *tilemap, int right, int down)
{
	char *new_map;
	int new_height, new_width;
	int i, j;
	int tilePos;
	int x, y;
	if (!tilemap) return;
	new_map = tilemap_new();
	if (!new_map) return;

	slog("Expanding tilemap!");

	// establish the new dimensions of the tilemap
	new_height = tilemap->height + down;
	new_width = tilemap->width + right;

	if ((down < 0) || (right < 0))
		slog("Shrinking the map!");

	if (new_height <= 0)
		new_height = 1;
	if (new_width <= 0)
		new_width = 1;

	// allocate space for the new tilemap's contents
	new_map = (char *)malloc(sizeof(char)*new_width*new_height + 1);
	if (!new_map) return;
	memset(new_map, 0, sizeof(char) * (new_width * new_height + 1));

	// copy over the old tilemap's contents
	slog("Old map! %s", tilemap->map);
	for (j = 0; j < new_height; j++)
	{
		for (i = 0; i < new_width; i++)
		{
			new_map[j * new_width + i] = '0';
		}
	}
	slog("Max memory address: %i", new_width*new_height + 1);
	for (j = 0; j < tilemap->height; j++)
	{
		for (i = 0; i < tilemap->width; i++)
		{
			//slog("Memory address: %i", j * tilemap->width + i);
			if (i >= new_width || j >= new_height)
			{
				continue;
			}
			if (j * new_width + i >= new_width*new_height)
			{
				slog("Address is too big!");
				continue;
			}
			slog("Writing to memory %i", j * new_width + i);
			new_map[j * new_width + i] = tilemap->map[j * tilemap->width + i];
		}
	}

	slog("New map! %s", new_map);
	slog("Free the map!");
	// free the old tilemap, and refresh the current tilemap
	free(tilemap->map);
	slog("Tilemap free!");
	tilemap->map = new_map;
	tilemap->height = new_height;
	tilemap->width = new_width;
	slog("Finished expanding tilemap!");
}

void tilemap_free(TileMap *tilemap)
{
	if (!tilemap)
		return;
	slog("setting the current tilemap");
	if (tilemap == get_current_tilemap())
		set_current_tilemap(NULL);
	slog("freeing the sprite");
	if (tilemap->tileset != NULL)
		gf2d_sprite_free(tilemap->tileset);
	slog("freeing the map");
	slog("reading the map %s", tilemap->map);
	if (tilemap->map != NULL)
		free(tilemap->map);
	slog("freeing the tilemap entirely");
	free(tilemap);
}

void draw_tile(Tile *t, float tile_width, int x, int y)
{
	// check if tile is on or near screen
	// if (........) return;
	gf2d_sprite_draw(
		t->sprite,
		vector2d(x * tile_width, y * tile_width),
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		t->frame
		);
}

void draw_map(Map *m)
{
	int i;
	int j;
	Tile *t;
	for (i = 0; i < 64; i++)
	{
		for (j = 0; j < 64; j++)
		{
			t = tile_get_by_id(m->tile[i][j]);
			draw_tile(t, 32, i, j);
		}
	}
}

void tilemap_load_walls(TileMap *tilemap, Vector2D position)
{
	int i, j, length;
	Entity *ent;
	char *wallname;
	if (!tilemap)return;
	if (!tilemap->map)return;
	if (!tilemap->tileset)return;
	for (j = 0; j < tilemap->height; j++)
	{
		for (i = 0; i < tilemap->width; i++)
		{
			if (tilemap->map[(j*tilemap->width) + i] != '0')
			{
				ent = entity_new();
				if (!ent)
				{
					slog("Unable to load collider for walls");
					return NULL;
				}
				ent->position = vector2d(position.x + (i * tilemap->tileset->frame_w), position.y + (j * tilemap->tileset->frame_h));
				ent->coll = box_collider_new();
				if (!ent->coll)
				{
					slog("Error allocating new collider");
					return NULL;
				}
				// find the biggest rectangle, try to minimize walls
				length = 0;
				while (tilemap->map[(j*tilemap->width) + i] != '0')
				{
					length++;
					i++;
					if (i >= tilemap->width) break;
				}
				ent->coll->width = tilemap->tileset->frame_w * length;
				ent->coll->height = tilemap->tileset->frame_h;
				ent->coll->parent = ent;
				sprintf(ent->name, "%s (%i)", "Wall", ((j*tilemap->width) + i));
				// don't bother, we need to draw the walkable tiles anyway
				//ent->sprite = tilemap->tileset;
				ent->layer = 1;
			}
		}
	}

	// connect walls
	entity_collapse_walls(tilemap);
}

TileMap *tilemap_load(char *filename, Vector2D position)
{
	TileMap *tilemap;
	Entity *ent, *prefab;
	FILE *file;
	int x, y;
	char buffer[1024];
	char spritefile[1024];
	int framewidth, frameheight, framesperline;
	int tileCtr = 0;
	int i;
	if (!filename)return NULL;
	file = fopen(filename, "r");
	if (!file)
	{
		slog("failed to open file %s", filename);
		return NULL;
	}
	tilemap = tilemap_new();
	if (!tilemap)
	{
		slog("failed to create a new tilemap");
		return NULL;
	}
	while (fscanf(file, "%s", buffer) != EOF)
	{
		if (strcmp(buffer, "width:") == 0)
		{
			fscanf(file, "%ui", &tilemap->width);
			continue;
		}
		if (strcmp(buffer, "height:") == 0)
		{
			fscanf(file, "%ui", &tilemap->height);
			continue;
		}
		if (strcmp(buffer, "start:") == 0)
		{
			fscanf(file, "%lf, %lf", &tilemap->start.x, &tilemap->start.y);
			continue;
		}
		if (strcmp(buffer, "end:") == 0)
		{
			fscanf(file, "%lf, %lf", &tilemap->end.x, &tilemap->end.y);
			continue;
		}
		if (strcmp(buffer, "tileset:") == 0)
		{
			fscanf(file, "%s %i,%i,%i", spritefile, &framewidth, &frameheight, &framesperline);
			if (strlen(spritefile) > 0)
			{
				tilemap->tileset = gf2d_sprite_load_all(spritefile, framewidth, frameheight, framesperline);
			}
			continue;
		}
		if (strcmp(buffer, "map_begin") == 0)
		{
			if ((tilemap->width * tilemap->height) == 0)
			{
				slog("width and height need be defined before starting the tile map");
				tilemap_free(tilemap);
				fclose(file);
				return NULL;
			}
			tilemap->map = (char *)malloc(sizeof(char) * (tilemap->width * tilemap->height + 1));
			if (!tilemap->map)
			{
				slog("failed to allocate data for map tile data");
				tilemap_free(tilemap);
				fclose(file);
				return NULL;
			}
			memset(tilemap->map, 0, sizeof(char) * (tilemap->width * tilemap->height + 1));
			while (fscanf(file, "%s", buffer) != EOF)
			{
				if (buffer[0] == '#')
				{
					continue;
				}
				if (strcmp("map_end", buffer) == 0)
				{
					break;
				}
				strcat(tilemap->map, buffer);
			}
		}
		if (strcmp(buffer, "entity:") == 0)
		{
			fscanf(file, "%s", buffer);
			prefab = config_loader_get_prefab_by_name(buffer);
			if (prefab == NULL)
			{
				slog("unable to find prefab");
			}
			ent = entity_new();
			entity_copy_prefab(ent, prefab);
			if (ent->init)
				ent->init(ent);
			else
				slog("cannot find an init function: %s", buffer);
			if (ent->take_damage != NULL)
				tilemap->numEnemies += 1;
			while (fscanf(file, "%s", buffer) != EOF)
			{
				if (buffer[0] == '#')
				{
					continue;
				}
				if (strcmp("entity_end", buffer) == 0)
				{
					break;
				}
				if (strcmp("position:", buffer) == 0)
				{
					fscanf(file, "%i,%i", &x, &y);
					ent->position = vector2d(position.x + (x * tilemap->tileset->frame_w), position.y + (y * tilemap->tileset->frame_h));
				}
				if (strcmp("retreat:", buffer) == 0)
				{
					fscanf(file, "%i,%i", &x, &y);
					ent->retreat = vector2d(x, y);
				}
				if (strcmp(buffer, "patrol:") == 0)
				{
					i = 0;
					while (strcmp(buffer, "patrol_end") != 0)
					{
						if (i <= 3)
						{
							fscanf(file, "%i,%i", &x, &y);
							ent->patrol[i] = vector2d(x, y);
						}
						else
							slog("ERROR: cannot support more than 4 patrol locations. discarding position %i", i);
						fscanf(file, "%s", buffer);
						i++;
					}
					ent->currentDestination = 0;
					if (i - 1 <= 3)
						ent->numPatrol = i - 1;
					else
						ent->numPatrol = 3;
				}
			}
		}
	}
	fclose(file);
	set_current_tilemap(tilemap);
	tilemap->position = position;
	return tilemap;
}

void tilemap_draw_walkable(TileMap *tilemap, Vector2D position)
{
	int i, j;
	if (!tilemap)return;
	if (!tilemap->map)return;
	if (!tilemap->tileset)return;
	for (j = 0; j < tilemap->height; j++)
	{
		for (i = 0; i < tilemap->width; i++)
		{
			if (tilemap->map[(j * tilemap->width) + i] != '0')
				continue;
			gf2d_sprite_draw(
				tilemap->tileset,
				vector2d(position.x + (i * tilemap->tileset->frame_w), position.y + (j * tilemap->tileset->frame_h)),
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				tilemap->map[(j * tilemap->width) + i] - '0');
		}
	}
}

void tilemap_draw_walls(TileMap *tilemap, Vector2D position)
{
	int i, j;
	if (!tilemap)return;
	if (!tilemap->map)return;
	if (!tilemap->tileset)return;
	for (j = 0; j < tilemap->height; j++)
	{
		for (i = 0; i < tilemap->width; i++)
		{
			if (tilemap->map[(j * tilemap->width) + i] == '0')
				continue;
			gf2d_sprite_draw(
				tilemap->tileset,
				vector2d(position.x + (i * tilemap->tileset->frame_w), position.y + (j * tilemap->tileset->frame_h)),
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				tilemap->map[(j * tilemap->width) + i] - '0');
		}
	}
}