#ifndef __INVENTORY_H__
#define __INVENTORY_H__

#include "gfc_list.h"

#define MAX_ITEMS 128

typedef enum
{
	II_ITEM1,
	II_ITEM2,
	II_ITEM3,
	II_ITEM4,
	II_ITEM5,
	II_ITEMMAX
}ItemIndinces;

typedef struct
{
	int weight;
	int weightLimit;
	GFC_List *itemList;
} Inventory;

void inventory_init(Inventory *inventory);

void inventory_close(Inventory *inventory);

void inventory_add_item(Inventory *inventory, const char *name);

#endif
