#ifndef __LEVEL_H__
#define __LEVEL_H__

#include "gf2d_sprite.h"

typedef struct
{
	Sprite*		background;
	Sprite*		tileSet;
	Uint8*		tileMap;
	Uint32		width, height;
} Level;

/**
* @brief spawn a player
* return NULL on error, a pointer to the player otherwise
**/
Level* level_new();

/**
* @brief
* @param
* @param
* @param
* @param
* @param
**/
void level_create(const char* background,
	const char* tileSet,
	Uint32 tileWidth,
	Uint32 tileHeight,
	Uint32 tilesPerLine,
	Uint32 width,
	Uint32 height);

/**
* @brief spawn a player
* return NULL on error, a pointer to the player otherwise
**/
void level_free(Level* free);

/**
* @brief spawn a player
* return NULL on error, a pointer to the player otherwise
**/
void level_draw(Level* level);

#endif
