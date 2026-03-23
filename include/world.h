#ifndef __WORLD_H__
#define __WORLD_H__

#include "gfc_shape.h"
#include "gf2d_sprite.h"

typedef struct
{
	Sprite		*background;	/* background image for the world */
	Sprite		*tileLayer;		/* pre-rendered tile layer */	
	Sprite		*tileSet;		/* sprite containing tiles for the world */
	Uint8		*tileMap;		/* the tiles that make up the world */
	Uint32		tileWidth;		/* how many tiles wide the map is */
	Uint32		tileHeight;		/* how many tiles tall the map is */
	Uint32      tileCount;		/* number of tiles for physics layer */
	GFC_Rect	*physicsLayer;	/* bounding boxes for collision testing on world */
} World;

/**
* @brief load the world from a config file
* @param filename: the name of the world file to load
* @return NULL on error, a usable world otherwise
*/
World *world_load(const char *filename);

/**
 * @brief allocate a new empty world
 * @param width: how many tiles wide the map is
 * @param height: how many tiles tall the map is
 * @return NULL on error, a blank world otherwise
 */
World *world_new(Uint32 width, Uint32 height);

/**
 * @brief free a previously allocated world
 * @param world: the world to free
 */
void world_free(World *world);

/**
 * @brief draw the world
 * @param world: the world to draw
 */
void world_draw(World *world);

/**
* @brief set the camera for the world
* @param world: the world set up the camera in
*/
void world_setup_camera(World *world);

#endif