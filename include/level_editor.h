#ifndef __LEVEL_EDITOR_H__
#define __LEVEL_EDITOR_H__

#include <SDL.h>
#include "gfc_text.h"
#include "gfc_list.h"
#include "gfc_shape.h"
#include "gf2d_sprite.h"
#include "level.h"

typedef enum
{
	MSS_NONE,
	MSS_ENTITY,
	MSS_TILE,
	MSS_LIGHT,
	MSS_ERASE,
	MSS_MAX
} MouseSelectState;

typedef struct
{
	GFC_TextLine	editor_name;	/* menu name of entity */
	GFC_TextLine	name;			/* system name of entity */
	Sprite			*sprite;		/* menu image of entity */
} EntData;

typedef struct
{
	Level			*level;			/* level that is being made by the level editor */
	Uint32			select;			/* the operation currently selected */
	Uint32			width;			/* width of the level */
	Uint32			height;			/* height of the level */
	Uint8			ent;			/* number corresponding to entity */
	Uint8			tile;			/* number corresponding to tile */
	Uint8			light;			/* light radius */
	GFC_Vector2D	pos;			/* the current placing position */
	GFC_List		*ent_list;		/* list of entity menu data */
	SDL_Texture		*bg;			/* texture representing the level background */
	SDL_Texture		*world;			/* texture representing the level the level editor will make */
} LevelEditor;

/*
* @brief creates a collection of level editor data to be used by the editor menu
* @return NULL on error, a level editor otherwise
*/
LevelEditor *level_editor_new();

/*
* @brief get the width data from the level editor
* @param level_editor: the editor to get the data from
* @return the current level width
*/
Uint32 level_editor_get_width(LevelEditor *level_editor);

/*
* @brief increase the width for the level editor
* @param level_editor: the level editor to modify
*/
void level_editor_increase_width(LevelEditor *level_editor);

/*
* @brief decrease the width for the level editor
* @param level_editor: the level editor to modify
*/
void level_editor_decrease_width(LevelEditor *level_editor);

/*
* @brief get the height data from the level editor
* @param level_editor: the editor to get the data from
* @return the current level height
*/
Uint32 level_editor_get_height(LevelEditor *level_editor);

/*
* @brief increase the height for the level editor
* @param level_editor: the level editor to modify
*/
void level_editor_increase_height(LevelEditor *level_editor);

/*
* @brief decrease the height for the level editor
* @param level_editor: the level editor to modify
*/
void level_editor_decrease_height(LevelEditor *level_editor);

/*
* @brief get the ent data from the level editor
* @param level_editor: the editor to get the data from
* @return the current ent in ent_list
*/
EntData *level_editor_get_ent(LevelEditor *level_editor);

/*
* @brief select the next entity for the level editor
* @param level_editor: the level editor to modify
*/
void level_editor_next_entity(LevelEditor *level_editor);

/*
* @brief select the previous entity for the level editor
* @param level_editor: the level editor to modify
*/
void level_editor_previous_entity(LevelEditor *level_editor);

/*
* @brief get the tile data from the level editor
* @param level_editor: the editor to get the data from
* @return the current tile index
*/
Uint8 level_editor_get_tile(LevelEditor *level_editor);

/*
* @brief select the next tile for the level editor
* @param level_editor: the level editor to modify
*/
void level_editor_next_tile(LevelEditor *level_editor);

/*
* @brief select the previous tile for the level editor
* @param level_editor: the level editor to modify
*/
void level_editor_previous_tile(LevelEditor *level_editor);

/*
* @brief get the light radius data from the level editor
* @param level_editor: the editor to get the data from
* @return the current light radius
*/
Uint8 level_editor_get_light(LevelEditor *level_editor);

/*
* @brief increase the light radius for the level editor
* @param level_editor: the level editor to modify
*/
void level_editor_increase_light(LevelEditor *level_editor);

/*
* @brief decrease the light radius for the level editor
* @param level_editor: the level editor to modify
*/
void level_editor_decrease_light(LevelEditor *level_editor);

/*
* @brief move the current placing position to the left
* @param level_editor: the level editor to modify
*/
void level_editor_pos_left(LevelEditor *level_editor);

/*
* @brief move the current placing position to the right
* @param level_editor: the level editor to modify
*/
void level_editor_pos_right(LevelEditor *level_editor);

/*
* @brief move the current placing position upward
* @param level_editor: the level editor to modify
*/
void level_editor_pos_up(LevelEditor *level_editor);

/*
* @brief move the current placing position downard
* @param level_editor: the level editor to modify
*/
void level_editor_pos_down(LevelEditor *level_editor);

/*
* @brief change the selected mouse operation
* @param level_editor: the level editor to modify
*/
void level_editor_change_select(LevelEditor *level_editor, Uint32 select);

/*
* brief add to the level based on the current mouse selection
* @param level_editor: the level editor to add to
*/
void level_editor_apply_select(LevelEditor *level_editor);

/*
* @brief update the level texture for the level editor
* @param level_editor: the level editor to modify
*/
void level_editor_update_texture(LevelEditor *level_editor);

/*
* @brief draw the current level state for the level editor
* @param bounds: the position and size of the level state render
*/
void level_editor_draw(LevelEditor *level_editor, GFC_Rect rect);

/*
* @brief free level editor data
* @param level_editor: the level editor to modify
* @param level_editor: the level editor data to be freed
*/
void level_editor_free(LevelEditor *level_editor);

#endif