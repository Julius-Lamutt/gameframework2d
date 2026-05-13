#ifndef __WORLD_OBJECT_H__
#define __WORLD_OBJECT_H__

#include "entity.h"
#include "world.h"

/**
* @brief spawn a world object
* @param world: the world the world object is in
* @param position: where to spawn the world object
* @param item_name: the name of the world object to spawn
* return NULL on error, a world object otherwise
*/
Entity *world_object_new(World *world, GFC_Vector2D position, const char *object_name);

/*
* @brief tell whether light world object is on
* @param self: the world object to check
* @return 0 if off, 1 if on, and 2 if not light world object
*/
Uint8 *world_object_light_on(Entity *self);

/*
* @brief trigger the light world object
* @note: will not do anything to non-light world_objects
* @param self: the world object to check
*/
void world_object_light_trigger(Entity *self);

#endif
