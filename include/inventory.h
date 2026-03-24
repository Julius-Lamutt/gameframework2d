#ifndef __INVENTORY_H__
#define __INVENTORY_H__

#include "gfc_list.h"
#include "items.h"

#define MAX_ITEMS 128

typedef enum
{
	II_ITEM1,
	II_ITEM2,
	II_ITEM3,
	II_ITEM4,
	II_ITEM5,
	II_ITEMMAX
} ItemIndinces;

typedef struct
{
	int weight;
	int weightLimit;
	GFC_List *itemList;
} Inventory;

/*
* @brief intilialize the inventory
* @param filename: the name of the json file to load
*/
void inventory_init(Inventory *inventory);

/*
* @brief free the inventory and all of its items
* @param inventory: the inventory to be freed
*/
void inventory_close(Inventory *inventory);

/*
* @brief get an inventory item by name
* @param inventory: the inventory that is being checked for an item
* @param name: name of the item
* @return NULL if item is not found, a pointer to the item otherwise
*/
Item *inventory_get_item_by_name(Inventory *inventory, const char *name);

/*
* @brief add an item to the inventory
* @param inventory: the inventory where the item will be added
* @param name: the name of the item to be added
*/
void inventory_add_item(Inventory *inventory, const char *name);

/*
* @brief remove an item from the inventory
* @param inventory: the inventory the item will be removed from
* @param name: the name of the item to be removed
*/
void inventory_remove_item(Inventory *inventory, const char *name);

#endif
