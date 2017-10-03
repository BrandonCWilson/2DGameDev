#include "map.h"
#include "tile.h"
#include "gf2d_sprite.h"

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