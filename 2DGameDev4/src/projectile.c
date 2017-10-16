#include "projectile.h"
#include "gf2d_draw.h"
#include "simple_logger.h"

void arrow_update(Entity *self)
{
	self->timer += 1;
	if (self->timer >= 360)
	{
		entity_free(self);
	}
}

void arrow_touch(Entity *self, Entity *other)
{

}

void arrow_init(Entity *self)
{

}