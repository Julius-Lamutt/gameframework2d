#ifndef __LEVEL_H__
#define __LEVEL_H__

#include "simple_json.h"
#include "gfc_text.h"
#include "gfc_list.h"
#include "gfc_vector.h"
#include "gf2d_sprite.h"

typedef struct
{
	GFC_Vector2D	pos;	/* position relative to other tiles */
	Uint8			index;	/* the type of tile to use */
} LevelTile;

typedef struct
{
	GFC_Vector2D	pos;	/* tile position of light */
	float			rad;	/* radius of the light */
} LevelLight;

typedef struct
{
	GFC_Vector2D	pos;		/* pixel position of the entity */
	GFC_TextLine	name;		/* name of the entity */
	Sprite			*sprite;	/* sprite menu data for the entity */
} LevelEntity;

typedef struct
{
	Uint16			width;		/* width of level in tiles */
	Uint16			height;		/* height of level in tiles */
	GFC_List		*tiles;		/* list of tiles */
	GFC_List		*lights;	/* list of tiles */
	GFC_List		*entities;	/* list of tiles */
} Level;

/*
* @brief create a new level
* @param width: width of the level
* @param height: height of the level
* @return NULL on error, a new level otherwise
*/
Level *level_new(Uint16 width, Uint16 height);

/*
* @brief add a tile to the level
* @param level: the level to add the tile for
* @param pos: position of the tile
* @param index: index of tile to use
*/
void level_add_tile(Level *level, GFC_Vector2D pos, Uint8 index);

/*
* @brief add a light to the level
* @param level: the level to add the light for
* @param pos: position of the light
* @param rad: radius of the light
*/
void level_add_light(Level *level, GFC_Vector2D pos, float rad);

/*
* @brief add an entity to the level
* @param level: the level to add the entity for
* @param pos: position of the entity
* @param name: name of the entity
*/
void level_add_entity(Level *level, GFC_Vector2D pos, const char *name, Sprite *sprite);

/*
* @brief remove a tile from the level
* @param level: the level to delete the tile for
* @param pos: position of the tile to delete
*/
void level_delete_tile(Level *level, GFC_Vector2D pos);

/*
* @brief remove a light from the level
* @param level: the level to delete the light for
* @param pos: position of the light to delete
*/
void level_delete_light(Level *level, GFC_Vector2D pos);

/*
* @brief remove an entity from the level
* @param level: the level to delete the entity for
* @param pos: position of the entity to delete
*/
void level_delete_entity(Level *level, GFC_Vector2D pos);

/*
* @brief save the level as a json file
* @param level: the level to save
* @param name: name of the level
*/
void level_save(Level *level, const char *name);

/*
* @brief free level information
* @param level: the level to free
*/
void level_free(Level *level);

#endif
