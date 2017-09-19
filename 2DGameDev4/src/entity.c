#include "entity.h"
#include "simple_logger.h"

typedef struct
{
	Uint32 max_entities;
	Entity * ent_list;

}EntManager;

static EntManager entity_manager;

void entity_system_init(Uint32 max)
{
	int i;
	slog("initializing entity system..");
	if (!max)
	{
		slog("cannot initialize an entity system for zero entities!");
		return;
	}
	entity_manager.max_entities = max;
	entity_manager.ent_list = (Entity *)malloc(sizeof(Entity)*max);
	memset(entity_manager.ent_list, 0, sizeof(Entity)*max);

	slog("entity system initialized");
	atexit(entity_system_close);
}

Entity *entity_new()
{
	int i;
	slog("creating a new entity..");
	/*search for an unused entity address*/
	for (i = 0; i < entity_manager.max_entities; i++)
	{
		if (!entity_manager.ent_list[i].inUse)
		{
			entity_delete(&entity_manager.ent_list[i]);	// clean up the old data
			entity_manager.ent_list[i].inUse = true;	// set it to inUse
			return &entity_manager.ent_list[i];			// return address of this array element
		}
	}
	slog("error: out of entity addresses");
	return NULL;
}

void entity_delete(Entity *ent)
{
	if (!ent)return;
	memset(ent, 0, sizeof(Entity));	//clean up the data
}

void entity_free(Entity *ent)
{
	if (!ent)return;
	ent->inUse = false;
}

void entity_update_all()
{
	int i;
	for (i = 0; i < entity_manager.max_entities; i++)
	{
		if (entity_manager.ent_list[i].inUse)
		{
			
			vector2d_add(entity_manager.ent_list[i].position, entity_manager.ent_list[i].position, entity_manager.ent_list[i].velocity);

			// handle the entity think functions
			if (!entity_manager.ent_list[i].update)
			{
				// no update function ..
			}
			else 
			{
				entity_manager.ent_list[i].update(&entity_manager.ent_list[i]);
			}

			if (!entity_manager.ent_list[i].sprite)
			{
				// no sprite ..
			}
			else
			{
				gf2d_sprite_draw(
					entity_manager.ent_list[i].sprite,
					entity_manager.ent_list[i].position,
					&entity_manager.ent_list[i].scale,
					NULL,
					NULL,
					NULL,
					NULL,
					(int)entity_manager.ent_list[i].frame);

				entity_manager.ent_list[i].frame += 0.1;
				if (entity_manager.ent_list[i].frame > 16.0)entity_manager.ent_list[i].frame = 0;
			}
		}

	}
}

void entity_draw(Entity *ent)
{
	slog("drawing entity..");

	gf2d_sprite_draw(&ent->sprite, ent->position, &ent->scale, 0, 0, 0, 0, (int)ent->frame);
}

void entity_system_close()
{
	slog("closing entity system..");
}