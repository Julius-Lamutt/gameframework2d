#include "simple_logger.h"
#include "level.h"

/*
* @brief get a tile from the list of tiles
* @param level: the level to get the tile from
* @param pos: position of the tile to get
* @param del: if true then delete upon retrieval, if false do nothing
* @return NULL if not found, a level tile otherwise
*/
static LevelTile *level_get_tile(Level *level, GFC_Vector2D pos, Uint8 del);

/*
* @brief get a light from the list of lights
* @param level: the level to get the light from
* @param pos: position of the light to get
* @param del: if true then delete upon retrieval, if false do nothing
* @return NULL if not found, a level light otherwise
*/
static LevelLight *level_get_light(Level *level, GFC_Vector2D pos, Uint8 del);

/*
* @brief get an entity from the list of entities
* @param level: the level to get the entity from
* @param pos: position of the entity to get
* @param del: if true then delete upon retrieval, if false do nothing
* @return NULL if not found, a level entity otherwise
*/
static LevelEntity *level_get_entity(Level *level, GFC_Vector2D pos, Uint8 del);

Level *level_new(Uint16 width, Uint16 height)
{
	Level *level;
	SJson *temp;

	level = gfc_allocate_array(sizeof(Level), 1);
	if (!level)
	{
		slog("failed to allocate a level");
		return NULL;
	}

	// add prefix to set file location and suffix to distinguish custom levels from other levels
	/*
	GFC_TextLine prefix = "defs/maps/";
	const char *suffix = "_custom.json";
	strcat(prefix, name);
	strcat(prefix, suffix);
	*/

	// set defaults
	level->width = width;
	level->height = height;
	level->tiles = gfc_list_new();
	level->lights = gfc_list_new();
	level->entities = gfc_list_new();

	if (!level->tiles)
	{
		slog("failed to create a list of tile data");
		return NULL;
	}
	if (!level->lights)
	{
		slog("failed to create a list of light data");
		return NULL;
	}
	if (!level->entities)
	{
		slog("failed to create a list of entity data");
		return NULL;
	}
	return level;
}

void level_add_tile(Level *level, GFC_Vector2D pos, Uint8 index)
{
	LevelTile *tile;

	if (!level || !level->tiles) return;

	tile = gfc_allocate_array(sizeof(LevelTile), 1);
	if (!tile)
	{
		slog("failed to allocate tile data for the level");
		return;
	}
	tile->pos = pos;
	tile->index = index;

	level_delete_tile(level, pos);
	gfc_list_append(level->tiles, tile);
}

void level_add_light(Level *level, GFC_Vector2D pos, float rad)
{
	LevelLight *light;

	if (!level || !level->lights) return;

	light = gfc_allocate_array(sizeof(LevelLight), 1);
	if (!light)
	{
		slog("failed to allocate light data for the level");
		return;
	}
	light->pos = pos;
	light->rad = rad;

	level_delete_light(level, pos);
	gfc_list_append(level->lights, light);
}

void level_add_entity(Level *level, GFC_Vector2D pos, const char *name, Sprite *sprite)
{
	LevelEntity *entity;

	if (!level || !level->entities) return;

	entity = gfc_allocate_array(sizeof(LevelEntity), 1);
	if (!entity)
	{
		slog("failed to allocate tile data for the level");
		return;
	}
	entity->pos = pos;
	gfc_line_cpy(entity->name, name);
	entity->sprite = sprite;

	level_delete_entity(level, pos);
	gfc_list_append(level->entities, entity);
}

static LevelTile *level_get_tile(Level *level, GFC_Vector2D pos, Uint8 del)
{
	int i, c;
	LevelTile *tile;

	if (!level) return;

	c = gfc_list_get_count(level->tiles);
	for (i = 0; i < c; i++)
	{
		tile = gfc_list_get_nth(level->tiles, i);
		if (!tile) continue;
		if (tile->pos.x == pos.x && tile->pos.y == pos.y)
		{
			if (del) gfc_list_delete_nth(level->tiles, i);
			return tile;
		}
	}
	return NULL;
}

static LevelLight *level_get_light(Level *level, GFC_Vector2D pos, Uint8 del)
{
	int i, c;
	LevelLight *light;

	if (!level) return;

	c = gfc_list_get_count(level->lights);
	for (i = 0; i < c; i++)
	{
		light = gfc_list_get_nth(level->lights, i);
		if (!light) continue;
		if (light->pos.x == pos.x && light->pos.y == pos.y)
		{
			if (del) gfc_list_delete_nth(level->lights, i);
			return light;
		}
	}
	return NULL;
}

static LevelEntity *level_get_entity(Level *level, GFC_Vector2D pos, Uint8 del)
{
	int i, c;
	LevelEntity *entity;

	if (!level) return;

	c = gfc_list_get_count(level->entities);
	for (i = 0; i < c; i++)
	{
		entity = gfc_list_get_nth(level->entities, i);
		if (!entity) continue;
		if (entity->pos.x == pos.x && entity->pos.y == pos.y)
		{
			if (del) gfc_list_delete_nth(level->entities, i);
			return entity;
		}
	}
	return NULL;
}

void level_delete_tile(Level *level, GFC_Vector2D pos)
{
	LevelTile *tile;

	if (!level) return;
	tile = level_get_tile(level, pos, 1);
	if (tile) free(tile);
}

void level_delete_light(Level *level, GFC_Vector2D pos)
{
	LevelLight *light;

	if (!level) return;
	light = level_get_light(level, pos, 1);
	if (light) free(light);
}

void level_delete_entity(Level *level, GFC_Vector2D pos)
{
	LevelEntity *entity;

	if (!level) return;
	entity = level_get_entity(level, pos, 1);
	if (entity) free(entity);
}

void level_save(Level *level, const char *name)
{
	int i, j, c;
	LevelTile *tile;
	LevelLight *light;
	LevelEntity *entity;
	SJson *json, *wjson, *ejson, *temp, *ent, *outer_array, *inner_array;
	
	if (!level) return;

	// json init
	json = sj_object_new();
	wjson = sj_object_new();
	ejson = sj_object_new();

	// boilerplate level data
	temp = sj_new_str("images/backgrounds/museum.png");
	sj_object_insert(wjson, "background", temp);
	temp = sj_new_str("images/backgrounds/tileset.png");
	sj_object_insert(wjson, "tileSet", temp);
	temp = sj_new_int(128);
	sj_object_insert(wjson, "frame_w", temp);
	temp = sj_new_int(128);
	sj_object_insert(wjson, "frame_h", temp);
	temp = sj_new_int(1);
	sj_object_insert(wjson, "frames_per_line", temp);
	temp = sj_new_int(1);
	sj_object_insert(wjson, "keep_surface", temp);

	// world tile layer
	outer_array = sj_array_new();
	for (j = 0; j < level->height/128 + 1; j++)
	{
		inner_array = sj_array_new();
		for (i = 0; i < level->width/128 + 1; i++)
		{
			tile = level_get_tile(level, gfc_vector2d(i, j), 1);
			if (!tile) temp = sj_new_int(0);
			else
			{
				temp = sj_new_int(tile->index + 1);
				free(tile);
			}
			sj_array_append(inner_array, temp);
		}
		sj_array_append(outer_array, inner_array);
	}
	sj_object_insert(wjson, "tileMap", outer_array);

	// world shadow map
	outer_array = sj_array_new();
	for (j = 0; j < level->height/128 + 1; j++)
	{
		inner_array = sj_array_new();
		for (i = 0; i < level->width/128 + 1; i++)
		{
			light = level_get_light(level, gfc_vector2d(i, j), 1);
			if (!light) temp = sj_new_int(0);
			else
			{
				temp = sj_new_int(light->rad);
				free(light);
			}
			sj_array_append(inner_array, temp);
		}
		sj_array_append(outer_array, inner_array);
	}
	sj_object_insert(wjson, "shadowMap", outer_array);
	sj_object_insert(json, "world", wjson);

	// world entity spawning
	outer_array = sj_array_new();
	c = gfc_list_get_count(level->entities);
	for (i = 0; i < c; i++)
	{
		entity = gfc_list_get_nth(level->entities, i);
		if (!entity) continue;
		ent = sj_object_new();
		temp = sj_new_str(entity->name);
		sj_object_insert(ent, "name", temp);
		inner_array = sj_array_new();
		temp = sj_new_int(entity->pos.x);
		sj_array_append(inner_array, temp);
		temp = sj_new_int(entity->pos.y);
		sj_array_append(inner_array, temp);
		sj_object_insert(ent, "location", inner_array);
		sj_array_append(outer_array, ent);
	}
	sj_object_insert(json, "entities", outer_array);

	// save level to file
	sj_save(json, name);
	sj_free(json);
}

void level_free(Level *level)
{
	if (!level) return;
	gfc_list_foreach(level->tiles, (gfc_work_func*)free);
	gfc_list_foreach(level->lights, (gfc_work_func*)free);
	gfc_list_foreach(level->entities, (gfc_work_func*)free);
	gfc_list_delete(level->tiles);
	gfc_list_delete(level->lights);
	gfc_list_delete(level->entities);
	free(level);
}