#ifndef __PHYSICS_H__
#define __PHYSICS_H__

#include <SDL.h>
#include "gfc_vector.h"
#include "gfc_shape.h"
#include "light.h"

/*
* @breif initialize the physics system
*/
void physics_init();

/*
* @brief cleanup the physics system
*/
void physics_cleanup();

/*
* @brief update the physics system's world information (needed for world collision testing)
* @param tile_count: the number of tiles in the physics layer
* @param physics_layer: the collision boxes that make up the physics_layer
* @param lights: shadow map light information used to make a "light physics" layer
*/
void physics_update_world_data(Uint32 tile_count, GFC_Rect *physics_layer, Light *lights);

/*
* @brief update an entity's move state
* @param tile_count: the number of tiles to check collisions for
* @param physics_layer: the tiles for collision testing
* @param box: the bounding box of the entity
* @param velocity: the velocity of the entity
*/
void physics_update_move_state(GFC_Rect box, Uint32 *move_state);

/*
* @brief get an entity's velocity
* @param box: the bounding box of the entity
* @param velocity: the velocity of the entity
* @param fall_speed: the maximum fall speed
*/
void physics_get_velocity(GFC_Rect box, GFC_Vector2D *velocity, float *fall_speed);

/*
* @brief get an entity's velocity based on the given collision type
* @param box: the bounding box of the entity
* @param velocity: the velocity of the entity
* @param fall_speed: the maximum fall speed
* @param type: whether an object falls (0), sticks (1), or bounces (2) during a collision
* @param made_contact: becomes 1 if collision occurs, 0 otherwise
*/
void physics_get_velocity_based_on_collision(GFC_Rect box, GFC_Vector2D *velocity, float *fall_speed, Uint32 type, Uint8 *made_contact);

/*
* @brief determines if there is a wall between two points
* @param a: the first point
* @param b: the second point
* @return 1 if there is a wall, 0 otherwise
*/
Uint8 physics_wall_between_points(GFC_Vector2D a, GFC_Vector2D b);

/*
* @brief determines if object is in light
* @param box: the bounding box of the object
* @return 1 if in light, 0 otherwise
*/
Uint8 physics_object_in_light(GFC_Rect box);

#endif
