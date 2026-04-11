#ifndef __WORLD_OBJECT_H__
#define __WORLD_OBJECT_H__

#include "entity.h"

/**
* @brief spawn a world object
* @param position: where to spawn the world object
* @param item_name: the name of the world object to spawn
* return NULL on error, a world object otherwise
*/
Entity *world_object_new(GFC_Vector2D position, const char *object_name);

#endif
