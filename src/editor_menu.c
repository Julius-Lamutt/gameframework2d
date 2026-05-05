#include "simple_logger.h"
#include "editor_menu.h"
#include "level_editor.h"
#include "elements.h"

extern void game_exit_editor();

extern GFC_Color mouse_color;

typedef struct
{
	Window			*win;		/* the current window */
	Uint8			selected;	/* true if window is currently selected */
	LevelEditor		*editor;	/* editor data the menu is manipulating */
} EditorMenuData;

/*
* @brief update the editor menu
* @param win: the current window
* @param updates: the list of elements to be updated
* @return 0 on error or no update, 1 otherwise
*/
int editor_menu_update(Window *win, GFC_List *updates);

/*
* @brief draw the editor menu
* @param win: the current window
* @return 0 on error or no draw, 1 otherwise
*/
int editor_menu_draw(Window *win);

/*
* @brief close the editor menu
* @param win: the current window
* @return 0 if handled, 1 otherwise
*/
int editor_menu_free(Window *win);

int editor_menu_update(Window *win, GFC_List *updates)
{
	Element *element;
	EditorMenuData *data;
	int i, c;

	if (!win) return 0;
	if (!updates) return 0;
	data = (EditorMenuData*)win->data;
	if (!data) return 0;

	c = gfc_list_get_count(win->elements);
	for (i = 0; i < c; i++)
	{
		element = gfc_list_get_nth(win->elements, i);
		if (!element) continue;
		if (gfc_strlcmp(element->name, "label_width_number") == 0)
		{
			GFC_TextWord buffer;
			_itoa(level_editor_get_width(data->editor), &buffer, 10);
			element_update_label(element, buffer);
		}
		else if (gfc_strlcmp(element->name, "label_height_number") == 0)
		{
			GFC_TextWord buffer;
			_itoa(level_editor_get_height(data->editor), &buffer, 10);
			element_update_label(element, buffer);
		}
		else if (gfc_strlcmp(element->name, "label_light_number") == 0)
		{
			GFC_TextWord buffer;
			_itoa(level_editor_get_light(data->editor), &buffer, 10);
			element_update_label(element, buffer);
		}
		else if (gfc_strlcmp(element->name, "actor_entity") == 0)
		{
			EntData *ent_data;
			ent_data = level_editor_get_ent(data->editor);
			
			element_update_actor(element, ent_data->sprite, 0);
		}
		else if (gfc_strlcmp(element->name, "actor_tile") == 0)
		{
			element_update_actor(element, NULL, level_editor_get_tile(data->editor));
		}
	}

	c = gfc_list_get_count(updates);
	for (i = 0; i < c; i++)
	{
		element = gfc_list_get_nth(updates, i);
		if (!element) continue;
		if (gfc_strlcmp(element->name, "button_return_to_menu") == 0)
		{
			window_free(win);
			game_exit_editor();
			return 1;
		}
		else if (gfc_strlcmp(element->name, "button_decrease_width") == 0)
		{
			level_editor_decrease_width(data->editor);
			return 1;
		}
		else if (gfc_strlcmp(element->name, "button_increase_width") == 0)
		{
			level_editor_increase_width(data->editor);
			return 1;
		}
		else if (gfc_strlcmp(element->name, "button_decrease_height") == 0)
		{
			level_editor_decrease_height(data->editor);
			return 1;
		}
		else if (gfc_strlcmp(element->name, "button_increase_height") == 0)
		{
			level_editor_increase_height(data->editor);
			return 1;
		}
		else if (gfc_strlcmp(element->name, "button_left_entity") == 0)
		{
			level_editor_previous_entity(data->editor);
			return 1;
		}
		else if (gfc_strlcmp(element->name, "button_right_entity") == 0)
		{
			level_editor_next_entity(data->editor);
			return 1;
		}
		else if (gfc_strlcmp(element->name, "button_left_tile") == 0)
		{
			level_editor_previous_tile(data->editor);
			return 1;
		}
		else if (gfc_strlcmp(element->name, "button_right_tile") == 0)
		{
			level_editor_next_tile(data->editor);
			return 1;
		}
		else if (gfc_strlcmp(element->name, "button_decrease_light") == 0)
		{
			level_editor_decrease_light(data->editor);
			return 1;
		}
		else if (gfc_strlcmp(element->name, "button_increase_light") == 0)
		{
			level_editor_increase_light(data->editor);
			return 1;
		}
		else if (gfc_strlcmp(element->name, "button_entity") == 0)
		{
			mouse_color = gfc_color8(30, 30, 30, 255);
			level_editor_change_select(data->editor, MSS_ENTITY);
			return 1;
		}
		else if (gfc_strlcmp(element->name, "button_tile") == 0)
		{
			mouse_color = gfc_color8(30, 220, 30, 255);
			level_editor_change_select(data->editor, MSS_TILE);
			return 1;
		}
		else if (gfc_strlcmp(element->name, "button_light") == 0)
		{
			mouse_color = gfc_color8(30, 30, 220, 255);
			level_editor_change_select(data->editor, MSS_LIGHT);
			return 1;
		}
		else if (gfc_strlcmp(element->name, "button_eraser") == 0)
		{
			mouse_color = gfc_color8(220, 220, 220, 255);
			level_editor_change_select(data->editor, MSS_ERASE);
			return 1;
		}
		else if (gfc_strlcmp(element->name, "button_save") == 0)
		{
			return 1;
		}
	}
	return 1;
}

int editor_menu_draw(Window *win)
{
	EditorMenuData *data;

	if (!win) return 0;
	if (!win->data) return 0;
	data = win->data;
	return 1;
}

int editor_menu_free(Window *win)
{
	EditorMenuData *data;

	if ((!win) || (!win->data)) return 0;
	data = (EditorMenuData*) win->data;
	level_editor_free(data->editor);
	free(data);
	return 0;
}

Window *editor_menu()
{
	Window *win;
	EditorMenuData *data;
	win = window_load("menus/editor.json");
	if (!win)
	{
		slog("failed to load editor menu");
		return NULL;
	}
	win->update = editor_menu_update;
	win->draw = editor_menu_draw;
	win->free =editor_menu_free;
	data = (EditorMenuData*)gfc_allocate_array(sizeof(EditorMenuData), 1);
	win->data = data;
	data->win = win;
	data->selected = 1;
	data->editor = level_editor_new();
	return win;
}

