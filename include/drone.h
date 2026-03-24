#ifndef __DRONE_H__
#define __DRONE_H__

#include "entity.h"

/**
* @brief spawn a drone
* return NULL on error, a pointer to a drone otherwise
*/
Entity *drone_new(Entity *owner);

#endif
