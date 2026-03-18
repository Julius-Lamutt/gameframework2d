#include "simple_logger.h"
#include "windows.h"

typedef struct
{
	Window  *window_list;
	Uint32	window_max;
} WindowManager;

static WindowManager _window_manager = {0};

/*
* @brief close the window system
*/
void window_system_close();

/*
* @brief run the update function for this window
* @param self: the entity to update
*/
void window_update(Window *win);

/*
* @breif run the draw function for this window
* @param self: the entity to draw
*/
void window_draw(Window *win);

void window_system_init(Uint32 max)
{
	if (_window_manager.window_list)
	{
		slog("window manager is aleady active");
		return;
	}
	if (!max)
	{
		slog("cannot initialize window system with zero windows");
		return;
	}
	_window_manager.window_list = gfc_allocate_array(sizeof(Window), max);
	if (!_window_manager.window_list)
	{
		slog("failed to allocate global window list");
		return;
	}
	_window_manager.window_max = max;
	atexit(window_system_close);
	slog("window system initialized");
}

void window_system_close()
{
	int i;
	for (i = 0; i < _window_manager.window_max; i++)
	{
		window_free(&_window_manager.window_list[i]);
	}
	free(_window_manager.window_list);
	memset(&_window_manager, 0, sizeof(WindowManager));
	slog("window system closed");
}

Window *window_new()
{
	int i;
	if (!_window_manager.window_list)
	{
		slog("window system has not been initialized!");
		return NULL;
	}
	for (i = 0; i < _window_manager.window_max; i++)
	{
		if (_window_manager.window_list[i]._inuse) continue;
		memset(&_window_manager.window_list[i], 0, sizeof(Window));
		_window_manager.window_list[i]._inuse = 1;
		return &_window_manager.window_list[i];
	}
	slog("no more available windows");
	return NULL;
}

void window_free(Window *win)
{
	if (!win) return;
	win->_inuse = 0; // save this spot for future windows

	// anything else we allocate for our window would get cleaned up here
	if (win->free) win->free(win);
}

void window_update(Window* win)
{
	if (!win) return;
	if (win->update) win->update(win, win->elements);
}

void window_system_update()
{
	int i;
	for (i = 0; i < _window_manager.window_max; i++)
	{
		if (!_window_manager.window_list[i]._inuse) continue;
		window_update(&_window_manager.window_list[i]);
	}
}

void window_draw(Window* win)
{
	if (!win) return;
	if (win->draw) win->draw(win);
}

void window_system_draw()
{
	int i;
	for (i = _window_manager.window_max - 1; i >= 0; i--)
	{
		if (!_window_manager.window_list[i]._inuse) continue;
		window_draw(&_window_manager.window_list[i]);
	}
}
