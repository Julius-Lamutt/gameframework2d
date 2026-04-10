#ifndef __PROJECTILES_H__
#define __PROJECTILES_H__

#include "entity.h"

/**
* @brief spawn a projectile
* @param owner: the entity that shot the projectile
* @param dir: the direction the projectile should travel
* @param proj_name: name of the projectile to spawn
* return NULL on error, a projectile otherwise
*/
Entity *projectile_new(Entity *owner, GFC_Vector2D dir, const char *proj_name);

#endif
