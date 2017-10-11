#include "raycast.h"
#include "simple_logger.h"

int get_line_intersection(Vector2D Apos, Vector2D Adir, Vector2D Bpos, Vector2D Bdir, Vector2D *out)
{
	float s1_x, s1_y, s2_x, s2_y;
	float s, t;
	float sDenom, tDenom;
	s1_x = Adir.x - Apos.x;     s1_y = Adir.y - Apos.y;
	s2_x = Bdir.x - Bpos.x;     s2_y = Bdir.y - Bpos.y;
	sDenom = (-s2_x * s1_y + s1_x * s2_y);
	tDenom = (-s2_x * s1_y + s1_x * s2_y);
	if ((!sDenom) || (!tDenom))
		return 0;
	s = (-s1_y * (Apos.x - Bpos.x) + s1_x * (Apos.y - Bpos.y)) / sDenom;
	t = (s2_x * (Apos.y - Bpos.y) - s2_y * (Apos.x - Bpos.x)) / tDenom;

	if (s >= 0 && s <= 1 && t >= 0 && t <= 1)
	{
		// Collision detected
		if (out != NULL)
		{
			out->x = Apos.x + (t * s1_x);
			out->y = Apos.y + (t * s1_y);
		}
		return 1;
	}
	return 0;
}

void raycasthit_free(RaycastHit *hit)
{
	if (!hit)
		return;
	free(hit);
}

RaycastHit *raycasthit_new()
{
	RaycastHit *hit;
	hit = (RaycastHit *)malloc(sizeof(RaycastHit));
	if (!hit)
		return NULL;
	memset(hit, 0, sizeof(RaycastHit));
	return hit;
}

RaycastHit *raycast(Vector2D start1, Vector2D direction1, Vector2D start2, Vector2D direction2)
{
	Vector2D hit;
	RaycastHit *rtn;
	rtn = raycasthit_new();
	if (!rtn)
	{
		slog("unable to allocate raycasthit");
		return NULL;
	}
	if (get_line_intersection(start1, direction1, start2, direction2, &hit) == 1)
	{
		rtn->hitpoint = hit;
		return rtn;
	}
	return NULL;
}