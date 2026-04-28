#ifndef __SHADOW_MAP_H__
#define __SHADOW_MAP_H__

#include <SDL.h>
#include "gf2d_sprite.h"
#include "gfc_shape.h"

typedef struct
{
	GFC_Rect	shadow_layer;
	SDL_Texture	*shadow_mask;
} ShadowMap;

/*
* @brief create a shadow map
* @param dimensions: the dimensions of the shadow map
* @return NULL on error, a shadow map otherwise
*/
ShadowMap *shadow_map_new(GFC_Rect dimensions);

/*
* @brief draw the shadow map
* @param shadow_map: the shadow map to draw
*/
void shadow_map_draw(ShadowMap *shadow_map);

/*
* @brief free the shadow map
* @param dhadow_map: the shadow map to be freed
*/
void shadow_map_free(ShadowMap *shadow_map);

#endif
