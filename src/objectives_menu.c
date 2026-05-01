#include "simple_logger.h"
#include "gfc_input.h"
#include "elements.h"
#include "font.h"
#include "objectives_menu.h"

typedef struct
{
	Window	*win;				/* the current window */
	Uint8	selected;			/* true if window is currently selected */
	int		item_pickup;
	int		environment_used;
	int		diamond_stolen;
} ObjectivesMenuData;

/*
* @brief update the main menu
* @param win: the current window
* @param update_elements: the list of elements to be updated
* @return 0 on error or no update, 1 otherwise
*/
int objectives_menu_update(Window *win, GFC_List *update_elements);

/*
* @brief draw the main menu
* @param win: the current window
* @return 0 on error or no draw, 1 otherwise
*/
int objectives_menu_draw(Window *win);

/*
* @brief close the main menu
* @param win: the current window
* @return 0 if handled, 1 otherwise
*/
int objectives_menu_free(Window *win);

void objective_complete(Window *win, int objective)
{
	ObjectivesMenuData *data;

	if (!win) return 0;
	if (!win->data) return 0;
	data = win->data;

	if (objective == 1) data->item_pickup = 1;
	if (objective == 2) data->environment_used = 1;
	if (objective == 3) data->diamond_stolen = 1;
}

int objectives_menu_update(Window *win, GFC_List *elements)
{
	Element *element;
	ObjectivesMenuData *data;
	int i, c, mx, my;

	if (!win) return 0;
	if (!elements) return 0;
	data = (ObjectivesMenuData*)win->data;
	if (!data) return 0;
	SDL_GetMouseState(&mx, &my);
	c = gfc_list_get_count(elements);
	for (i = 0; i < c; i++)
	{
		element = gfc_list_get_nth(elements, i);
		if (!element) continue;
		if (gfc_strlcmp(element->name, "label_objective_1") == 0)
		{
			if (data->item_pickup) element_update_label(element, "Completed!!");
		}
		if (gfc_strlcmp(element->name, "label_objective_2") == 0)
		{
			if (data->environment_used) element_update_label(element, "Completed!!");
		}
		if (gfc_strlcmp(element->name, "label_objective_3") == 0)
		{
			if (data->diamond_stolen) element_update_label(element, "Completed!!");
		}
	}
	return 1;
}

int objectives_menu_draw(Window *win)
{
	ObjectivesMenuData *data;

	if (!win) return 0;
	if (!win->data) return 0;
	data = win->data;
	return 1;
}

int objectives_menu_free(Window *win)
{
	ObjectivesMenuData *data;

	if ((!win) || (!win->data)) return 0;
	data = (ObjectivesMenuData*) win->data;
	free(data);
	return 0;
}

Window *objectives_menu()
{
	Window *win;
	ObjectivesMenuData *data;
	win = window_load("menus/objectives.json");
	if (!win)
	{
		slog("failed to load main_menu");
		return NULL;
	}
	win->update = objectives_menu_update;
	win->draw = objectives_menu_draw;
	win->free = objectives_menu_free;
	data = (ObjectivesMenuData*)gfc_allocate_array(sizeof(ObjectivesMenuData), 1);
	win->data = data;
	data->win = win;
	data->item_pickup = 0;
	data->environment_used = 0;
	data->diamond_stolen = 0;
	data->selected = 1;
	return win;
}