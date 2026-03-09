#ifndef __COLLISION_H__
#define __COLLISION_H__

#include "entity.h"

/**
* @brief test to see if two entities have collided
* @param self the entity testing for collision
* @param other the entity being tested for collision with the first entity
* @return true if collision, false otherwise
**/
Bool collision(Entity* self, Entity* other);

#endif
