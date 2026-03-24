#include "simple_logger.h"
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

	if ((!inventory) || (!name)) return;
	item = inventory_get_item_by_name(inventory, name);
	if (item)
	{
		item->count++;
		return;
	}
	item = item_new(name);
	if (!item) return;
	gfc_list_append(inventory->itemList, item);
}

void inventory_remove_item(Inventory *inventory, const char *name)
{
	Item *item;

	if ((!inventory) || (!name)) return;
	item = inventory_get_item_by_name(inventory, name);
	if (item)
	{
		item->count--;
	}
	else
	{
		slog("item %s not in inventory", name);
	}
	if (item->count == 0)
	{
		if (!gfc_list_delete_data(inventory->itemList, item)) slog("failed delete of item %s", item);
		item_free(item);
	}
	return;
}