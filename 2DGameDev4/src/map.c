#include "map.h"
#include "tile.h"
#include "gf2d_sprite.h"
#include "simple_logger.h"
#include "entity.h"
#include <stdio.h>

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

void tilemap_free(TileMap *tilemap)
{
	if (!tilemap)return;
	if (tilemap->map)
	{
		free(tilemap->map);
	}
	gf2d_sprite_free(tilemap->tileset);
	free(tilemap);
}

void tilemap_load_walls(TileMap *tilemap, Vector2D position)
{
	int i, j;
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
				ent->coll->width = tilemap->tileset->frame_w;
				ent->coll->height = tilemap->tileset->frame_h;
				ent->coll->parent = ent;
				sprintf(ent->name, "%s (%i)", "Wall", ((j*tilemap->width) + i));
				ent->sprite = tilemap->tileset;
			}
		}
	}
}

TileMap *tilemap_load(char *filename)
{
	TileMap *tilemap;
	Entity *ent;
	FILE *file;
	char buffer[1024];
	char spritefile[1024];
	int framewidth, frameheight, framesperline;
	int tileCtr = 0;
	if (!filename)return NULL;
	file = fopen(filename, "r");
	if (!file)
	{
		slog("failed to open file %s", filename);
		return NULL;
	}
	tilemap = (TileMap *)malloc(sizeof(TileMap));
	if (!tilemap)
	{
		slog("failed to allocate tile map data");
		return NULL;
	}
	memset(tilemap, 0, sizeof(TileMap));
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
			tilemap->map = (char *)malloc(sizeof(char) * tilemap->width * tilemap->height);
			if (!tilemap->map)
			{
				slog("failed to allocate data for map tile data");
				tilemap_free(tilemap);
				fclose(file);
				return NULL;
			}
			memset(tilemap->map, 0, sizeof(char) * tilemap->width * tilemap->height);
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
			continue;
		}
	}
	fclose(file);
	return tilemap;
}

void tilemap_draw(TileMap *tilemap, Vector2D position)
{
	int i, j;
	if (!tilemap)return;
	if (!tilemap->map)return;
	if (!tilemap->tileset)return;
	for (j = 0; j < tilemap->height; j++)
	{
		for (i = 0; i < tilemap->width; i++)
		{
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
	gf2d_sprite_draw(
		tilemap->tileset,
		vector2d(position.x + (tilemap->start.x * tilemap->tileset->frame_w), position.y + (tilemap->start.y * tilemap->tileset->frame_h)),
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		2);
	gf2d_sprite_draw(
		tilemap->tileset,
		vector2d(position.x + (tilemap->end.x * tilemap->tileset->frame_w), position.y + (tilemap->end.y * tilemap->tileset->frame_h)),
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		3);
}