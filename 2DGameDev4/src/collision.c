#include "collision.h"
#include "simple_logger.h"

BoxCollider *box_collider_new()
{
	BoxCollider *rtn;
	rtn = (BoxCollider *)malloc(sizeof(BoxCollider));
	if (!rtn)
	{
		slog("Unable to allocate box collider data");
		return NULL;
	}
	memset(rtn, 0, sizeof(BoxCollider));
	return rtn;
}

void box_collider_free(BoxCollider *box)
{
	if (!box)
		return;
	free(box);
}