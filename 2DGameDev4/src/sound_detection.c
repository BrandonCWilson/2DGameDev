#include "sound_detection.h"
#include "simple_logger.h"
#include <SDL2_gfxPrimitives.h>
#include "gf2d_graphics.h"
#include "entity.h"
#include "player.h"

typedef struct SonarManager_s
{
	Sonar *sonarlist;
	int max;
} SonarManager;

SonarManager sonar_manager;

void sonar_delete(Sonar *self)
{
	memset(self, 0, sizeof(Sonar));
}

void sonar_clear_all()
{
	int i;
	for (i = 0; i < sonar_manager.max; i++)
	{
		sonar_delete(&sonar_manager.sonarlist[i]);
	}
}

void sonar_close()
{
	if (sonar_manager.sonarlist == NULL)
		return;
	sonar_clear_all();
	free(sonar_manager.sonarlist);
}

void sonar_init(Uint32 max)
{
	if (!max)
	{
		slog("Cannot initialize a sonar system for 0 blips!");
		return NULL;
	}

	sonar_manager.sonarlist = (Sonar *)malloc(sizeof(Sonar)*max);
	if (!sonar_manager.sonarlist)
	{
		slog("Failed to allocate memory for the sonar list!");
		return NULL;
	}
	memset(sonar_manager.sonarlist, 0, sizeof(Sonar)*max);
	sonar_manager.max = max;
	atexit(sonar_close);
}

void sonar_draw_all()
{
	int i;

	for (i = 0; i < sonar_manager.max; i++)
	{
		if (sonar_manager.sonarlist[i].inUse)
		{
			filledCircleRGBA(gf2d_graphics_get_renderer(),
				sonar_manager.sonarlist[i].center.x - (get_player() == NULL ? 0 : get_player()->position.x - 600),
				sonar_manager.sonarlist[i].center.y - (get_player() == NULL ? 0 : get_player()->position.y - 360),
				sonar_manager.sonarlist[i].radius / (25 - sonar_manager.sonarlist[i].timer <= 0 ? 1 : 25 - sonar_manager.sonarlist[i].timer),
				0,
				0,
				0,
				sonar_manager.sonarlist[i].noise);
		}
	}
}

void sonar_update()
{
	int i;
	Sonar *tmp;

	for (i = 0; i < sonar_manager.max; i++)
	{
		if (sonar_manager.sonarlist[i].inUse)
		{
			tmp = &sonar_manager.sonarlist[i];
			tmp->timer += 1;
			if (tmp->timer == 1)
			{
				tmp->decrement = tmp->noise / 60;

				// alert anyone within earshot
				entity_alert_within_radius(tmp->center, tmp->radius);
			}

			tmp->noise -= tmp->decrement;
			if (tmp->noise < 0)
				tmp->noise = 0;

			if (tmp->timer >= 60)
				sonar_free(tmp);
		}
	}
}

void sonar_free(Sonar *sonar)
{
	if (!sonar) return;
	sonar->inUse = false;
}

Sonar *sonar_new()
{
	int i;

	for (i = 0; i < sonar_manager.max; i++)
	{
		if (sonar_manager.sonarlist[i].inUse == false)
		{
			sonar_delete(&sonar_manager.sonarlist[i]);
			sonar_manager.sonarlist[i].inUse = true;
			return &sonar_manager.sonarlist[i];
		}
	}
	return NULL;
}