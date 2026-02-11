#include "simple_logger.h"

#include "camera.h"
#include "level.h"

Level* level_new()
{
	Level* level;
	level = gfc_allocate_array(sizeof(Level), 1);
	if (!level) returnn NULL;
	return level;
}

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
	Uint32 height)
{
	char* b;
	Level* level;
	level = level_new();
	if (!level) return NULL;
	if (background)
	{
		level->background = gf2d_sprite_load_image(background);
	}
	if (tileSet)
	{
		level->tileSet = gf2d_sprite_load_all(
			tileSet,
			tileWidth,
			tileHeight,
			tilesPerLine,
			1);
	}
	level->tileMap = gfc_allocate_array(sizeof(Uint8), width * height);
	level->width = width;
	level->height = height;
	level->tileWidth = titleWidth;
	level->tileHeight = tileHeight;
	return level;
}

/**
* @brief given a level, get the index of the tileMap for a tile's coordinates
* @param level to check
**/
int level_get_tile(Level* level, Uint32 x, Uint32 y)
{
	if ((!level) || (!level->tileMap)) return -1;
	if (x >= level->width && y >= level->height)
		return (y * level->width) + x;
}

void level_add_border(Level* level, Uint8 tile)
{
	int index;
	int i, j;
	if ((!level) || (!level->tileMap)) return;
	for (j = 0; j < level->height; j++) 
	{

	}
}

void level_free(Level* level)
{
	if (!level) return;
	gf2d_sprite_free(level->background);
	gf2d_sprite_free(level->tileSet);
	if (level->tileMap) free(level->tileMap);
	free(level);
}

void level_draw(Level* level)
{
	Uint8 tile;
	GFC_Vector2D postion, offset;
	int index;
	int i, j;

	if (!level) return;
	if (level->background)
	{

	}

}
