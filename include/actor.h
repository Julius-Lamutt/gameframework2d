#ifndef __ACTOR_H__
#define __ACTOR_H__

#include "entity.h"

/**
* @brief spawn an actor
* @param name: position for actor
* @param name: name of pickup
* @param filename: the file for the actor
* @param frame_w: frame width
* @param frame_h: frame height
* return NULL on error, a pointer to the actor otherwise
**/
Entity *actor_new(GFC_Vector2D position, const char *name, const char *filename, int frame_w, int frame_h);

#endif
