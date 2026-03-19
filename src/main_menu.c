#include "simple_logger.h"
#include "elements.h"
#include "main_menu.h"

typedef struct
{
	Window	*win;		/* the current window */
	Uint8	selected;	/* true if window is currently selected */
} MainMenuData;

/*
* @brief update the main menu
* @param win: the current window
* @param update_elements: the list of elements to be updated
* @return 0 on error or no update, 1 otherwise
*/
int main_menu_update(Window *win, GFC_List *update_elements);

/*
* @brief draw the main menu
* @param win: the current window
* @return 0 on error or no draw, 1 otherwise
*/
int main_menu_draw(Window *win);

/*
* @brief close the main menu
* @param win: the current window
* @return 0 if handled, 1 otherwise
*/
int main_menu_free(Window *win);

int main_menu_update(Window *win, GFC_List *update_elements)
{
	Element *element;
	MainMenuData *data;

	if (!win) return 0;
	if (!update_elements) return 0;
	data = (MainMenuData*)win->data;
	if (!data) return 0;
	return 1;
}

int main_menu_draw(Window *win)
{
	MainMenuData *data;

	if (!win) return 0;
	if (!win->data) return 0;
	data = win->data;
	return 1;
}

int main_menu_free(Window *win)
{
	MainMenuData *data;

	if ((!win) || (!win->data)) return 0;
	data = (MainMenuData*) win->data;
	free(data);
	return 0;
}

Window *main_menu()
{
	Window *win;
	MainMenuData *data;
	win = window_load("menus/main_menu.json");
	if (!win)
	{
		slog("failed to load main_menu");
		return NULL;
	}
	win->update = main_menu_update;
	win->draw = main_menu_draw;
	win->free = main_menu_free;
	data = (MainMenuData*)gfc_allocate_array(sizeof(MainMenuData), 1);
	win->data = data;
	data->win = win;
	data->selected = 1;
	return win;
}