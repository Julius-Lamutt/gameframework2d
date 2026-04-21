#ifndef __PHYSICS_H__
#define __PHYSICS_H__

#include <SDL.h>
#include "gfc_vector.h"
#include "gfc_shape.h"

/*
* @brief update an entity's move state
* @param tile_count: the number of tiles to check collisions for
* @param physics_layer: the tiles for collision testing
* @param box: the bounding box of the entity
* @param velocity: the velocity of the entity
*/
void physics_update_move_state(Uint32 tile_count, GFC_Rect *physics_layer, GFC_Rect box, Uint32 *move_state);

/*
* @brief update an entity's velocity
* @param tile_count: the number of tiles to check collisions for
* @param physics_layer: the tiles for collision testing
* @param box: the bounding box of the entity
* @param velocity: the velocity of the entity
* @param fall_speed: the maximum fall speed
*/
void physics_update_velocity(Uint32 tile_count, GFC_Rect *physics_layer, GFC_Rect box, GFC_Vector2D *velocity, float fall_speed);

#endif
