#ifndef __COLLISION_H__
#define __COLLISION_H__

#include "gfc_shape.h"

/**
* @brief test to see if one entity will collide with another entity
* @param self_box: the collision box for the first entity
* @param self_velocity: the velocity of the first entity
* @param other_box: the colllison box for the second entity
* @param other_velocity: the velocity of the second entity
* @return true if collision, false otherwise
*/
Bool collide_with_entity(GFC_Rect self_box, GFC_Vector2D self_velocity, GFC_Rect other_box, GFC_Vector2D other_velocity);

/**
* @brief test to see if one entity will collide with another entity
* @param self_box: the collision box for the first entity
* @param self_velocity: the velocity of the first entity
* @param other_box: the colllison box for the second entity
* @param other_velocity: the velocity of the second entity
* @return for both x and y: true if collision, false otherwise
*/
GFC_Vector2D collide_with_entity_vector(GFC_Rect self_box, GFC_Vector2D self_velocity, GFC_Rect other_box, GFC_Vector2D other_velocity);

/**
* @brief test to see if an entity will collide with the world
* @param tile_count: the number of tiles to check collisions for
* @param physics_layer: the tiles for collision testing
* @param box: the bounding box of the entity
* @param velocity: the velocity of the entity
* @return for both x and y: true if collision, false otherwise
*/
GFC_Vector2D collide_with_world(Uint32 tile_count, GFC_Rect *physics_layer, GFC_Rect box, GFC_Vector2D velocity);

/**
* @brief test to see where an entity is on a world tile floor or ceiling or is falling
* @param tile_count: the number of tiles to check collisions for
* @param physics_layer: the tiles for collision testing
* @param box: the bounding box of the entity
* @return 0 if ceiling, 1 if floor, and 2 if falling
*/
Uint8 collide_with_world_floor_or_ceiling(Uint32 tile_count, GFC_Rect *physics_layer, GFC_Rect box);

/**
* @brief test to see if an entity is on a world object floor or ceiling or is falling
* @param self_box: the bounding box of the entity
* @param other_box: the bounding box of the object
* @return 0 if ceiling, 1 if floor, and 2 if falling
*/
Uint8 collide_with_object_floor_or_ceiling(GFC_Rect self_box, GFC_Rect other_box);

#endif
