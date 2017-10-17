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
	if (!self || !other)
		return;
	if (other->layer == 1)
	{
		if (self->die != NULL)
			self->die(self);
	}
	if ((other->layer == 3) || (other->layer == 2))
	{
		if (self->parent != NULL)
		{
			if (other->layer == self->parent->layer)
				return;
		}
		if (other->take_damage != NULL)
			other->take_damage(other, self->damage);
		self->die(self);
	}
}

void arrow_init(Entity *self)
{

}

void arrow_die(Entity *self)
{
	entity_free(self);
}