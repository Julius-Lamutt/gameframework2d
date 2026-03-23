#ifndef __PHYSICS_H__
#define __PHYSICS_H__

#include <SDL.h>

/*
* @brief initializes the physics system
* @param frame_rate: the expected frame rate used for calculating delta time
*/
void physics_system_init(float frame_rate);

/*
* @breif update delta time for physics system
*/
void physics_update_delta();

#endif
