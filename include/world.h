#ifndef __WORLD_H__
#define __WORLD_H__

#include "gf2d_sprite.h"

typedef struct
{
	Sprite	*background;	/**<background image for the world*/
	Sprite  *tileLayer;		/**<pre-rendered tile layer*/	
	Sprite  *tileSet;		/**<sprite containing tiles for the world*/
	Uint8   *tileMap;		/**<the tiles that make up the world*/
	Uint32	tileWidth;		/**<how many tiles wide the map is*/
	Uint32  tileHeight;		/**<how many tiles tall the map is*/
} World;


/**
* @brief build the tile layer for the world
* @param world the world to build tile layer on
*/
void world_build_tile_layer(World* world);

/**
* @brief test function to see if world works
* @return NULL on error, a blank world otherwise
*/
World* world_test_new();

/**
 * @brief allocate a new empty world
 * @param width how many tiles wide the map is
 * @param height how many tiles tall the map is
 * @return NULL on error, a blank world otherwise
 */
World *world_new(Uint32 width, Uint32 height);

/**
 * @brief free a previously allocated world
 * @param world the world to free
 */
void world_free(World *world);

/**
 * @brief draw the world
 * @param world the world to draw
 */
void world_draw(World *world);

#endif