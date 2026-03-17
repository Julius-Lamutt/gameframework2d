#ifndef __ITEMS_H__
#define __ITEMS_H__

#include <SDL.h>
#include "gfc_text.h"
#include "gf2d_sprite.h"
#include "simple_json.h"

typedef struct
{
	GFC_TextLine name;
	Sprite *sprite;
	int count; /*how many of the item I have*/
	int max; /*how many of the item I can carry*/
} Item;

/*
* @brief intilialize and load item definitions
* @param filename: the name of the json file to load
*/
void items_init(const char *filename);

/*
* @brief get an item definition by its name
* @param name: the name of the item
* @return NULL if not found, otherwise the definition of the item
* @Note: DO NOT FREE THAT DATA
*/
SJson *items_get_def_by_name(const char *name);

Item *item_new(const char *name);

void item_free(Item *item);

#endif

