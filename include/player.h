#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "entity.h"

/**
* @brief spawn a player
* @param position: where to spawn the player
* @return NULL on error, a player otherwise
*/
Entity *player_new(GFC_Vector2D position);

#endif