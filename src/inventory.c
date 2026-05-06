#include "simple_logger.h"
#include "inventory_menu.h"
#include "items.h"
#include "inventory.h"

void inventory_init(Inventory* inventory)
{
	if (!inventory) return;
	inventory->itemList = gfc_list_new();
}

void inventory_close(Inventory* inventory)
{
	if (!inventory) return;
	gfc_list_foreach(inventory->itemList, (gfc_work_func*)item_free);
	gfc_list_delete(inventory->itemList);
	inventory->itemList = NULL;
}

Item* inventory_get_item_by_name(Inventory* inventory, const char* name)
{
	int i, c;
	Item *item;
	if (!inventory) return NULL;
	c = gfc_list_get_count(inventory->itemList);
	for (i = 0; i < c; i++)
	{
		item = gfc_list_get_nth(inventory->itemList, i);
		if (!item) continue;
		if (gfc_strlcmp(item->name, name) == 0) return item; // found it
	}
	slog("no item found by name '%s'", name);
	return NULL;
}

void inventory_add_item(Inventory* inventory, const char* name)
{
	Item *item;
	Window *win;

	if ((!inventory) || (!name)) return;

	win = window_find_by_name("inventory");

	item = inventory_get_item_by_name(inventory, name);
	if (item)
	{
		item->count++;
		if (gfc_strlcmp(item->name, "tool_shuriken") == 0)
		{
			inventory_menu_update_shuriken(win, item->count);
		}
		else if (gfc_strlcmp(item->name, "tool_teleporter") == 0)
		{
			inventory_menu_update_teleport(win, item->count);
		}
		else if (gfc_strlcmp(item->name, "tool_smoke") == 0)
		{
			inventory_menu_update_smoke(win, item->count);
		}
		else if (gfc_strlcmp(item->name, "tool_jump") == 0)
		{
			inventory_menu_update_jump(win, item->count);
		}
		return;
	}
	item = item_new(name);
	if (!item) return;
	gfc_list_append(inventory->itemList, item);

	if (gfc_strlcmp(item->name, "tool_shuriken") == 0)
	{
		inventory_menu_update_shuriken(win, 1);
	}
	else if (gfc_strlcmp(item->name, "tool_teleporter") == 0)
	{
		inventory_menu_update_teleport(win, 1);
	}
	else if (gfc_strlcmp(item->name, "tool_smoke") == 0)
	{
		inventory_menu_update_smoke(win, 1);
	}
	else if (gfc_strlcmp(item->name, "tool_jump") == 0)
	{
		inventory_menu_update_jump(win, 1);
	}
}

void inventory_remove_item(Inventory *inventory, const char *name)
{
	Item *item;

	if ((!inventory) || (!name)) return;
	item = inventory_get_item_by_name(inventory, name);
	if (item)
	{
		Window *win;

		win = window_find_by_name("inventory");

		item->count--;
		if (gfc_strlcmp(item->name, "tool_shuriken") == 0)
		{
			inventory_menu_update_shuriken(win, item->count);
		}
		else if (gfc_strlcmp(item->name, "tool_teleporter") == 0)
		{
			inventory_menu_update_teleport(win, item->count);
		}
		else if (gfc_strlcmp(item->name, "tool_smoke") == 0)
		{
			inventory_menu_update_smoke(win, item->count);
		}
		else if (gfc_strlcmp(item->name, "tool_jump") == 0)
		{
			inventory_menu_update_jump(win, item->count);
		}
	}
	else
	{
		slog("item %s not in inventory", name);
	}
	if (item->count == 0)
	{
		if (gfc_list_delete_data(inventory->itemList, item)) slog("failed delete of item %s", item);
		item_free(item);
	}
	return;
}