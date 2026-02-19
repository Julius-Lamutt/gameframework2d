#ifndef  __BULLET_H__
#define  __BULLET_H__

#include <SDL.h>
#include "entity.h"

/**
* @brief spawn a bullet
* @param owner - the one who shot the bullet
* @param pos - spawn position of the bullet
* return NULL on error, a pointer to the player otherwise
*/
Entity* bullet_new(Entity* owner, GFC_Vector2D pos);

#endif
