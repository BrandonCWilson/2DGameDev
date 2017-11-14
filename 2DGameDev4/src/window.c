#include "window.h"

typedef struct
{
	Uint32 max_windows;
	Window *win_list;
} WinManager;

static WinManager window_manager;

void window_system_init(Uint32 max)
{
	int i;
	slog("initializing window system..");
	if (!max)
	{
		slog("cannot initialize a window system for zero windows!");
		return;
	}
	window_manager.max_windows = max;
	window_manager.win_list = (Window *)malloc(sizeof(Window)*max);
	memset(window_manager.win_list, 0, sizeof(Window)*max);

	slog("window system initialized");
	atexit(window_system_close);
}

void window_update_all()
{
	int i;
	for (i = 0; i < window_manager.max_windows; i++)
	{
		if (window_manager.win_list[i].inUse)
		{
			if (window_manager.win_list[i].update != NULL)
				window_manager.win_list[i].update(&window_manager.win_list[i]);
		}
	}
}

void window_draw_all()
{
	int i;
	for (i = 0; i < window_manager.max_windows; i++)
	{
		if (window_manager.win_list[i].inUse)
		{
			if (window_manager.win_list[i].sprite != NULL)
			{
				// draw the 9 sprites as necessary based on window dimensions, scale them accordingly
			}
		}
	}
}

Window *window_new()
{
	int i;
	/*search for an unused entity address*/
	for (i = 0; i < window_manager.max_windows; i++)
	{
		if (!window_manager.win_list[i].inUse)
		{
			entity_delete(&window_manager.win_list[i]);	// clean up the old data
			window_manager.win_list[i].inUse = true;	// set it to inUse
			return &window_manager.win_list[i];			// return address of this array element
		}
	}
	slog("error: out of entity addresses");
	return NULL;
}

void window_free(Window *win)
{
	if (!win)
		return;
	win->inUse = false;
}

void window_delete(Window *win)
{
	if (!win)
		return;
	if (win->sprite != NULL)
		gf2d_sprite_free(win->sprite);
	memset(win, 0, sizeof(Window));
}

void window_clear_all()
{
	int i;
	for (i = 0; i < window_manager.max_windows; i++)
	{
		window_delete(&window_manager.win_list[i]);
	}
}

void window_system_close()
{
	window_clear_all();
	if (window_manager.win_list != NULL)
	{
		free(window_manager.win_list);
	}
	window_manager.win_list = NULL;
	window_manager.max_windows = 0;
}