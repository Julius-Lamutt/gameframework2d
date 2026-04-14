#include "simple_logger.h"
#include "gfc_input.h"
#include "elements.h"
#include "main_menu.h"

extern void game_start();
extern void game_exit();

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
	int i, c, mx, my;

	if (!win) return 0;
	if (!update_elements) return 0;
	data = (MainMenuData*)win->data;
	if (!data) return 0;
	SDL_GetMouseState(&mx, &my);
	c = gfc_list_get_count(update_elements);
	for (i = 0; i < c; i++)
	{
		element = gfc_list_get_nth(update_elements, i);
		if (!element) continue;
		if (element->index == -1) continue;
		if (gfc_strlcmp(element->name, "button_start_demo") == 0)
		{
			if (element->state == ES_ACTIVE)
			{
				window_free(win);
				game_start();
				return 1;
			}
		}
		if (gfc_strlcmp(element->name, "button_quit_demo") == 0)
		{
			if (element->state == ES_ACTIVE) game_exit();
		}
		if (gfc_point_in_rect(gfc_vector2d(mx, my), element->bounds))
		{
			if (gfc_input_mouse_left_pressed()) element->state = ES_ACTIVE;
			else element->state = ES_HIGHLIGHT;
		}
		else element->state = ES_IDLE;
	}
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