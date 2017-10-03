#include "testUpdate.h"
#include "simple_logger.h"
#include "entity.h"

void clickerUpdate(Entity *self)
{
	self->timer += 1;
	if (self->timer%20==0)
	{
		self->velocity = vector2d_rotate(self->velocity, 3.14 * 0.25);
		//entity_delete(self);
	}
	if (self->timer > 1000)
	{
		entity_delete(self);
	}
}