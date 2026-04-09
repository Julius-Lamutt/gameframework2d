#ifndef __ITEM_PICKUP_H__
#define __ITEM_PICKUP_H__

#include "simple_json.h"
#include "entity.h"

/**
* @brief spawn an item pickup
* @param position: where to spawn the item pickpup
* @param item_name: the name of the item pickup to spawn
* return NULL on error, an item pickup otherwise
*/
Entity *item_pickup_new(GFC_Vector2D position, const char *item_name);

#endif
