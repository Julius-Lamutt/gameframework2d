#ifndef __SHADOW_MAP_H__
#define __SHADOW_MAP_H__

#include <SDL.h>
#include "gf2d_sprite.h"
#include "gfc_shape.h"
#include "light.h"

typedef struct
{
	SDL_Texture	*shadow_mask;	/* texture to be modulated onto render for shadowing */
	Light		*lights;		/* data for creating light in the shadow map */
	Uint16		world_w;		/* width in tiles of the world the shadow map is for */
	Uint16		world_h;		/* height in tiles of the world the shadow map is for */
	float		tile_size;		/* the width/height of each tile */
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
* @brief add a light to the shadow map
* @param shadow_map: the shadow map to add the light to
* @param light: the light to add
*/
void shadow_map_add_light(ShadowMap *shadow_map, Light *light);

/*
* @brief remove a light from the shadow map
* @param shadow_map: the shadow map to remove the light from
* @param light: the light to remove
*/
void shadow_map_remove_light(ShadowMap *shadow_map, Light *light);

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
