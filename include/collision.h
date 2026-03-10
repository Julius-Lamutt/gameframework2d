#ifndef __COLLISION_H__
#define __COLLISION_H__

#include "entity.h"
#include "world.h"

/**
* @brief test to see if one entity collides with another entity
* @param self: the entity testing for collision
* @param other: the entity being tested for collision by the first entity
* @return true if collision, false otherwise
**/
Bool collide_with_entity(Entity *self, Entity *other);

/**
* @brief test to see if an entity collides with the world
* @param self: the entity testing for collision
* @param world: the world being tested for collision by the entity
* @return true if collision, false otherwise
**/
Bool collide_with_world(Entity *self, World *world);

#endif
