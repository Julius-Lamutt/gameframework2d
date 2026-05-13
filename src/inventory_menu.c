#include "simple_logger.h"
#include "ai.h"
#include "inventory.h"
#include "elements.h"
#include "inventory_menu.h"

typedef struct
{
	Window	*win;		/* the current window */
	Uint8	selected;	/* true if window is currently selected */
	Uint8	shuriken;	/* inventory number of shurikens */
	Uint8	teleport;	/* inventory number of teleports */
	Uint8	smoke;		/* inventory number of smokes */
	Uint8	jump;		/* inventory number of jumps */
	Uint8	drone;		/* whether drone has focus or not */
} InvMenuData;

/*
* @brief update the inventory menu
* @param win: the current window
* @param updates: the list of elements to be updated
* @return 0 on error or no update, 1 otherwise
*/
int inventory_menu_update(Window *win, GFC_List *updates);

/*
* @brief draw the main menu
* @param win: the current window
* @return 0 on error or no draw, 1 otherwise
*/
int inventory_menu_draw(Window *win);

/*
* @brief close the main menu
* @param win: the current window
* @return 0 if handled, 1 otherwise
*/
int inventory_menu_free(Window *win);

void inventory_menu_update_shuriken(Window *win, Uint8 count)
{
	InvMenuData *data;

	if (!win) return 0;
	data = (InvMenuData*)win->data;
	if (!data) return 0;

	data->shuriken = count;
}

void inventory_menu_update_teleport(Window *win, Uint8 count)
{
	InvMenuData *data;

	if (!win) return 0;
	data = (InvMenuData*)win->data;
	if (!data) return 0;

	data->teleport = count;
}

void inventory_menu_update_smoke(Window *win, Uint8 count)
{
	InvMenuData *data;

	if (!win) return 0;
	data = (InvMenuData*)win->data;
	if (!data) return 0;

	data->smoke = count;
}

void inventory_menu_update_jump(Window *win, Uint8 count)
{
	InvMenuData *data;

	if (!win) return 0;
	data = (InvMenuData*)win->data;
	if (!data) return 0;

	data->jump = count;
}

void inventory_menu_update_drone(Window *win)
{
	InvMenuData *data;

	if (!win) return 0;
	data = (InvMenuData*)win->data;
	if (!data) return 0;

	if (data->drone == 0) data->drone = 1;
	else data->drone = 0;
}

int inventory_menu_update(Window *win, GFC_List *updates)
{
	Element *element;
	InvMenuData *data;
	int i, c;

	if (!win) return 0;
	if (!updates) return 0;
	data = (InvMenuData*)win->data;
	if (!data) return 0;

	c = gfc_list_get_count(win->elements);
	for (i = 0; i < c; i++)
	{
		element = gfc_list_get_nth(win->elements, i);
		if (!element) continue;
		if (gfc_strlcmp(element->name, "label_ai_status") == 0)
		{
			Uint32 status;
			GFC_Color color;

			status = ai_get_alert_state();
			if (status == 1)
			{
				color = gfc_color8(20, 220, 20, 255);
				element_update_label(element, "Normal", &color);
			}
			else if (status == 2)
			{
				color = gfc_color8(220, 220, 20, 255);
				element_update_label(element, "Caution", &color);
			}
			else if (status == 3)
			{
				color = gfc_color8(220, 20, 20, 255);
				element_update_label(element, "Alert", &color);
			}
		}
		else if (gfc_strlcmp(element->name, "label_ai_time") == 0)
		{
			GFC_TextWord buffer;
			int time;

			time = ai_get_caution_timer() / 1000;
			if (time == 0) element_update_label(element, " ", NULL);
			else
			{
				_itoa(time, &buffer, 10);
				element_update_label(element, buffer, NULL);
			}
		}
		else if (gfc_strlcmp(element->name, "label_shuriken") == 0)
		{
			GFC_TextWord buffer;

			_itoa((int)data->shuriken, &buffer, 10);
			element_update_label(element, buffer, NULL);
		}
		else if (gfc_strlcmp(element->name, "label_teleport") == 0)
		{
			GFC_TextWord buffer;

			_itoa((int)data->teleport, &buffer, 10);
			element_update_label(element, buffer, NULL);
		}
		else if (gfc_strlcmp(element->name, "label_smoke") == 0)
		{
			GFC_TextWord buffer;

			_itoa((int)data->smoke, &buffer, 10);
			element_update_label(element, buffer, NULL);
		}
		else if (gfc_strlcmp(element->name, "label_jump") == 0)
		{
			GFC_TextWord buffer;

			_itoa((int)data->jump, &buffer, 10);
			element_update_label(element, buffer, NULL);
		}
		else if (gfc_strlcmp(element->name, "actor_drone") == 0)
		{
			GFC_Color color;

			color = gfc_color8(255, 255, 255, 255);
			if (data->drone) element_update_actor(element, NULL, 0, &color);
			else
			{
				color = gfc_color8(150, 150, 150, 255);
				element_update_actor(element, NULL, 0, &color);
			}
		}
	}
}

int inventory_menu_draw(Window *win)
{
	InvMenuData *data;

	if (!win) return 0;
	if (!win->data) return 0;
	data = win->data;
	return 1;
}

int inventory_menu_free(Window *win)
{
	InvMenuData *data;

	if ((!win) || (!win->data)) return 0;
	data = (InvMenuData*) win->data;
	free(data);
	return 0;
}

Window *inventory_menu()
{
	Window *win;
	InvMenuData *data;
	win = window_load("menus/inventory.json");
	if (!win)
	{
		slog("failed to load inventory menu");
		return NULL;
	}
	win->update = inventory_menu_update;
	win->draw = inventory_menu_draw;
	win->free = inventory_menu_free;
	data = (InvMenuData*)gfc_allocate_array(sizeof(InvMenuData), 1);
	win->data = data;
	data->win = win;
	data->selected = 1;
	data->shuriken = 0;
	data->teleport = 0;
	data->smoke = 0;
	data->jump = 0;
	data->drone = 0;
	return win;
}