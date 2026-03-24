#ifndef __ITEM_PICKUP_H__
#define __ITEM_PICKUP_H__

#include "simple_json.h"
#include "entity.h"

/**
* @brief spawn an item pickup
* @param name: position for pickpup
* @param name: name of pickup
* @param filename: the file for the item pickup
* @param frame_w: frame width
* @param frame_h: frame height
* return NULL on error, a pointer to the item pickup otherwise
**/
Entity *item_pickup_new(GFC_Vector2D position, const char *name, const char *filename, int frame_w, int frame_h);

#endif
