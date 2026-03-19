#include "simple_logger.h"
#include "simple_json.h"
#include "elements.h"
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
		// set defaults
		_window_manager.window_list[i]._inuse = 1;
		_window_manager.window_list[i].hidden = 0;
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
		if ((!_window_manager.window_list[i]._inuse) || (_window_manager.window_list[i].hidden)) continue;
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
		if ((!_window_manager.window_list[i]._inuse) || (_window_manager.window_list[i].hidden)) continue;
		window_draw(&_window_manager.window_list[i]);
	}
}

Window *window_load(const char *filename)
{
	Window *win, *parent;
	SJson *file, *window, *element_list, *array;
	const char *name, *background, *border, *pname;
	GFC_Rect size, canvas;
	GFC_List *elements;

	if (!filename)
	{
		slog("no file name provided for window load");
		return NULL;
	}
	file = sj_load(filename);
	if (!file)
	{
		slog("failed to load window file '%s'", filename);
		return NULL;
	}
	window = sj_object_get_value(file, "window");
	if (!window)
	{
		sj_free(file);
		slog("missing window object in file '%s'", filename);
		return NULL;
	}
	
	name = sj_object_get_value_as_string(window, "name"); 
	if (!name)
	{
		sj_free(file);
		slog("missing window name object in file '%s'", filename);
		return NULL;
	}

	element_list = sj_object_get_value(window, "elements");
	if (!element_list)
	{
		sj_free(file);
		slog("missing window elements object in file '%s'", filename);
		return NULL;
	}
	elements = element_list_load(element_list);
	if (!elements)
	{
		sj_free(file);
		slog("failed to get window elements in file '%s'");
		return NULL;
	}
	
	background = sj_object_get_value_as_string(window, "background");
	if (!background)
	{
		sj_free(file);
		slog("missing window background object in file '%s'", filename);
		return NULL;
	}
	
	border = sj_object_get_value_as_string(window, "border");
	if (!border)
	{
		sj_free(file);
		slog("missing window border object in file '%s'", filename);
		return NULL;
	}
	
	array = sj_object_get_value(window, "size");
	if (!array)
	{
		sj_free(file);
		slog("missing window size object in file '%s'", filename);
		return NULL;
	}
	if (sj_array_get_count(array) != 4)
	{
		sj_free(file);
		slog("missing or extra window size dimensions in file '%s'", filename);
		return NULL;
	}
	if (!sj_get_float_value(sj_array_get_nth(array, 0), &size.x) ||
		!sj_get_float_value(sj_array_get_nth(array, 1), &size.y) ||
		!sj_get_float_value(sj_array_get_nth(array, 2), &size.w) ||
		!sj_get_float_value(sj_array_get_nth(array, 3), &size.h))
	{
		sj_free(file);
		slog("one or more size dimensions are invalid in file '%s'", filename);
		return NULL;
	}
	
	array = sj_object_get_value(window, "canvas");
	if (!array)
	{
		sj_free(file);
		slog("missing window canvas object in file '%s'", filename);
		return NULL;
	}
	if (sj_array_get_count(array) != 4)
	{
		sj_free(file);
		slog("missing or extra window canvas dimensions in file '%s'", filename);
		return NULL;
	}
	if (!sj_get_float_value(sj_array_get_nth(array, 0), &canvas.x) ||
		!sj_get_float_value(sj_array_get_nth(array, 1), &canvas.y) ||
		!sj_get_float_value(sj_array_get_nth(array, 2), &canvas.w) ||
		!sj_get_float_value(sj_array_get_nth(array, 3), &canvas.h))
	{
		sj_free(file);
		slog("one or more canvas dimensions are invalid in file '%s'", filename);
		return NULL;
	}

	pname = sj_object_get_value_as_string(window, "parent");
	if (!pname) parent = NULL;
	else parent = window_find_by_name(pname);

	sj_free(file);
	win = window_new();
	if (!win) return NULL;
	gfc_line_cpy(win->name, name);
	win->background = gf2d_sprite_load_image(background);
	win->border = gf2d_sprite_load_image(border);
	win->size = size;
	win->canvas = canvas;
	win->parent = parent;
	if (parent) parent->child = win;
	return win;
}

Window *window_find_by_name(const char* name)
{
	int i;

	if (!name)
	{
		slog("failed to get window name");
		return NULL;
	}
	for (i = 0; i < _window_manager.window_max; i++)
	{
		if (!_window_manager.window_list[i]._inuse) continue;
		if (gfc_strlcmp(_window_manager.window_list[i].name, name) == 0) return &_window_manager.window_list[i];
	}
	return NULL;
}