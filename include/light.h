#ifndef __LIGHT_H__
#define __LIGHT_H__

#include <SDL.h>
#include "gfc_vector.h"

/* NOTE: default radius is 64 pixels, use fractional radii for smaller light sources */
typedef struct
{
	GFC_Vector2D	pos;	// where to place the light
	float			r1;		// circle width radius of the light
	float			r2;		// circle height radius of the light
} Light;

/*
* @brief create light data to be used by the shadow map
* @param pos: where to place the light
* @param r1: the circle width radius of the light
* @param r2: the circle height radius of the light
*/
Light *light_new(GFC_Vector2D pos, float r1, float r2);

/*
* @brief free a light
* @param light: the light to be free
*/
void light_free(Light *light);

#endif
