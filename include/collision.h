#ifndef __COLLISION_H__
#define __COLLISION_H__

#include "gfc_shape.h"
#include "entity.h"
#include "world.h"

/**
* @brief test to see if one entity collides with another entity
* @param self: the entity testing for collision
* @param other: the entity being tested for collision by the first entity
* @return true if collision, false otherwise
*/
Bool collide_with_entity(Entity *self, Entity *other);

/**
* @brief test to see if an entity will collide with the world
* @param box: the bounding box of the entity
* @param tiles: the bounding boxes of the world physics layer
* @param velocity: the velocity of the entity
* @param tile_count: the number of tiles in the world physics layer
* @return for both x and y: true if collision, false otherwise
*/
GFC_Vector2D collide_with_world(GFC_Rect box, GFC_Vector2D velocity, World *world);

#endif
