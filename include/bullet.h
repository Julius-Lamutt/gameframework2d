#ifndef  __BULLET_H__
#define  __BULLET_H__

#include <SDL.h>
#include "entity.h"

typedef struct
{
	Entity* owner;
	Entity* victim;
	float   range;
} Bullet;

/**
* @brief spawn a bullet
* return NULL on error, a pointer to the player otherwise
*/
Entity* bullet_new();

#endif
