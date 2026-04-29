#ifndef __SHADOW_MAP_H__
#define __SHADOW_MAP_H__

#include <SDL.h>
#include "gf2d_sprite.h"
#include "gfc_shape.h"
#include "light.h"

typedef struct
{
	SDL_Texture	*shadow_mask;
	Light		*lights;
} ShadowMap;

/*
* @brief create a shadow map
* @param world_w: width of the world
* @param world_h: height of the world
* @param tile_size: width/height of a tile in the world
* @return NULL on error, a shadow map otherwise
*/
ShadowMap *shadow_map_new(Uint16 world_w, Uint16 world_h, float tile_size, Light *lights);

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
