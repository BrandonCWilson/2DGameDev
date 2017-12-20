#include "scrolls.h"
#include "simple_logger.h"
#include "gamestate.h"

void scroll_start_level(Scroll *self)
{
	PriorityNode *cursor;
	char *levelpath;
	int i;
	if (!self) return;
	i = 0;
	for (cursor = self->optionsList->head; cursor != NULL; cursor = cursor->next)
	{
		if (i == self->currentOption)
		{
			slog("loading level: %s", cursor->data);
			levelpath = (char *)malloc(sizeof(char)*(strlen(cursor->data) + strlen("levels/")));
			if (!levelpath)
			{
				slog("Failed to allocate space for a temporary string during level loading.");
				return;
			}
			strcpy(levelpath, "levels/");
			levelpath = strcat(levelpath, cursor->data);
			game_load_map(levelpath);
			//free(levelpath);
			main_menu_toggle();
			return;
		}
		else if (i > self->currentOption)
			break;
		i++;
	}
}