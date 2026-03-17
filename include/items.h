#ifndef __ITEMS_H__
#define __ITEMS_H__

#include "simple_json.h"

/*
* @brief intilialize and load item definitions
* @param filename: the name of the json file to load
*/
void items_initialize(const char *filename);

/*
* @brief get an item definition by its name
* @param name: the name of the item
* @return NULL if not found, otherwise the definition of the item
* @Note: DO NOT FREE THAT DATA
*/
SJson *items_get_def_by_name(const char *name);

#endif

