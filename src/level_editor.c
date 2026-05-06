#include "simple_logger.h"
#include "simple_json.h"
#include "gf2d_graphics.h"
#include "gf2d_draw.h"
#include "camera.h"
#include "level_editor.h"

/*
* @brief load the list of entity data for the level editor
* @return a gfc_list of entity data
*/
static GFC_List *level_editor_ent_list_load();

static GFC_List *level_editor_ent_list_load()
{
	int i, c;
	GFC_List *ent_list;
	SJson *json, *ejson, *ojson, *array;

	ent_list = gfc_list_new();
	if (!ent_list)
	{
		slog("failed to allocate a the level editor ent_list");
		return NULL;
	}
	json = sj_load("defs/ent_data.json");
	if (!json)
	{
		gfc_list_delete(ent_list);
		slog("failed to load ent data config file");
		return NULL;
	}
	ejson = sj_object_get_value(json, "entities");
	if (!ejson)
	{
		free(json);
		gfc_list_delete(ent_list);
		slog("missing ent data entities object");
		return NULL;
	}

	c = sj_array_get_count(ejson);
	for (i = 0; i < c; i++)
	{
		const char *editor_name, *name, *filename;
		Sint32 frame_w, frame_h, frames_per_line;
		Sprite *sprite;
		EntData *data;

		ojson = sj_array_get_nth(ejson, i);
		if (!ojson) continue;

		data = gfc_allocate_array(sizeof(EntData), 1);
		if (!data)
		{
			free(json);
			gfc_list_delete(ent_list);
			slog("failed to allocate ent data for an ent");
			return NULL;
		}

		editor_name = sj_object_get_value_as_string(ojson, "editor_name");
		if (!editor_name)
		{
			free(data);
			free(json);
			gfc_list_delete(ent_list);
			slog("missing ent editor_name object");
			return NULL;
		}

		name = sj_object_get_value_as_string(ojson, "name");
		if (!name)
		{
			free(data);
			free(json);
			gfc_list_delete(ent_list);
			slog("missing ent name object");
			return NULL;
		}

		array = sj_object_get_value(ojson, "sprite");
		if (!array)
		{
			free(data);
			free(json);
			gfc_list_delete(ent_list);
			slog("missing sprite object for player entity");
			return NULL;
		}
		if (sj_array_get_count(array) != 4)
		{
			free(data);
			free(json);
			gfc_list_delete(ent_list);
			slog("missing or extra sprite parameters for player entity");
			return NULL;
		}
		filename = sj_get_string_value(sj_array_get_nth(array, 0));
		if (!filename ||
			!sj_get_integer_value(sj_array_get_nth(array, 1), &frame_w) ||
			!sj_get_integer_value(sj_array_get_nth(array, 2), &frame_h) ||
			!sj_get_integer_value(sj_array_get_nth(array, 3), &frames_per_line))
		{
			free(data);
			free(json);
			gfc_list_delete(ent_list);
			slog("one or more sprite parameters are invalid for player entity");
			return NULL;
		}
		sprite = gf2d_sprite_load_all(filename, frame_w, frame_h, frames_per_line, 1);

		gfc_line_cpy(data->editor_name, editor_name);
		gfc_line_cpy(data->name, name);
		data->sprite = sprite;
		gfc_list_append(ent_list, data);
	}

	return ent_list;
}

LevelEditor *level_editor_new()
{
	LevelEditor *level_editor;
	SDL_Surface *bg_surf, *world_surf;
	GFC_Vector2D scale;
	Sprite *bg;

	level_editor = gfc_allocate_array(sizeof(LevelEditor), 1);
	if (!level_editor)
	{
		slog("failed to allocate level editor data");
		return NULL;
	}

	level_editor->level = level_new(1280, 1280);
	if (!level_editor->level)
	{
		slog("failed to allocate level for level editor");
		return NULL;
	}

	level_editor->ent_list = level_editor_ent_list_load();
	if (!level_editor->ent_list)
	{
		slog("failed to allocate ent data for level editor");
		return NULL;
	}
	
	// background texture
	bg_surf = gf2d_graphics_create_surface(1200, 720);
	bg = gf2d_sprite_load_all("images/backgrounds/museum.png", 600, 323, 1, 1);
	scale =  gfc_vector2d(2, 2.3);
	gf2d_sprite_draw_to_surface(bg, gfc_vector2d(0, 0), &scale, NULL, 0, bg_surf);
	level_editor->bg = SDL_CreateTextureFromSurface(gf2d_graphics_get_renderer(), bg_surf);

	// level texture
	world_surf = gf2d_graphics_create_surface(12800, 12800);
	level_editor->world = SDL_CreateTextureFromSurface(gf2d_graphics_get_renderer(), world_surf);

	// set defaults
	level_editor->select = MSS_NONE;
	level_editor->width = 1280;
	level_editor->height = 1280;
	level_editor->pos = gfc_vector2d(0, 0);
	level_editor->ent = 0;
	level_editor->tile = 0;
	level_editor->light = 1;
	return level_editor;
}

Uint32 level_editor_get_width(LevelEditor *level_editor)
{
	if (!level_editor) return -1;
	return level_editor->width;
}

void level_editor_increase_width(LevelEditor *level_editor)
{
	if (!level_editor) return;
	if (!(level_editor->width >= 12800)) level_editor->width += 128;
	level_editor->level->width = level_editor->width;
}

void level_editor_decrease_width(LevelEditor *level_editor)
{
	if (!level_editor) return;
	if (!(level_editor->width <= 1280)) level_editor->width -= 128;
	level_editor->level->width = level_editor->width;
	if (level_editor->pos.x > level_editor->width) level_editor->pos.x = level_editor->width;
}

Uint32 level_editor_get_height(LevelEditor *level_editor)
{
	if (!level_editor) return -1;
	return level_editor->height;
}

void level_editor_increase_height(LevelEditor *level_editor)
{
	if (!level_editor) return;
	if (!(level_editor->height >= 12800)) level_editor->height += 128;
	level_editor->level->height = level_editor->height;
}

void level_editor_decrease_height(LevelEditor *level_editor)
{
	if (!level_editor) return;
	if (!(level_editor->height <= 1280)) level_editor->height -= 128;
	level_editor->level->height = level_editor->height;
	if (level_editor->pos.y > level_editor->height) level_editor->pos.y = level_editor->height;
}

EntData *level_editor_get_ent(LevelEditor *level_editor)
{
	EntData *data;

	if (!level_editor || !level_editor->ent_list) return NULL;
	
	data = gfc_list_get_nth(level_editor->ent_list, level_editor->ent);
	if (!data)
	{
		slog("failed to get current ent data");
		return NULL;
	}
	return data;
}

void level_editor_next_entity(LevelEditor *level_editor)
{
	if (!level_editor) return;
	if (!(level_editor->ent >= gfc_list_get_count(level_editor->ent_list) - 1)) level_editor->ent++;
	else level_editor->ent = 0;
}

void level_editor_previous_entity(LevelEditor *level_editor)
{
	if (!level_editor) return;
	if (!(level_editor->ent <= 0)) level_editor->ent--;
	else level_editor->ent = gfc_list_get_count(level_editor->ent_list) - 1;
}

Uint8 level_editor_get_tile(LevelEditor *level_editor)
{
	if (!level_editor) return;
	return level_editor->tile;
}

void level_editor_next_tile(LevelEditor *level_editor)
{
	if (!level_editor) return;
	if (!(level_editor->tile >= 3)) level_editor->tile++;
	else level_editor->tile = 0;
}

void level_editor_previous_tile(LevelEditor *level_editor)
{
	if (!level_editor) return;
	if (!(level_editor->tile <= 0)) level_editor->tile--;
	else level_editor->tile = 3;
}

Uint8 level_editor_get_light(LevelEditor *level_editor)
{
	if (!level_editor) return;
	return level_editor->light;
}

void level_editor_increase_light(LevelEditor *level_editor)
{
	if (!level_editor) return;
	if (!(level_editor->light >= 25)) level_editor->light++;
}

void level_editor_decrease_light(LevelEditor *level_editor)
{
	if (!level_editor) return;
	if (!(level_editor->light <= 1)) level_editor->light--;
}

void level_editor_pos_left(LevelEditor *level_editor)
{
	if (!level_editor) return NULL;

	switch (level_editor->select)
	{
	case MSS_NONE:
		level_editor->pos.x -= 16;
		break;

	case MSS_ENTITY:
		level_editor->pos.x -= 16;
		break;

	case MSS_TILE:
		level_editor->pos.x -= 128;
		break;

	case MSS_LIGHT:
		level_editor->pos.x -= 128;
		break;

	case MSS_ERASE:
		level_editor->pos.x -= 16;
		break;
	}

	if (level_editor->pos.x < 0) level_editor->pos.x = 0;
}

void level_editor_pos_right(LevelEditor *level_editor)
{
	if (!level_editor) return NULL;

	switch (level_editor->select)
	{
	case MSS_NONE:
		level_editor->pos.x += 16;
		break;

	case MSS_ENTITY:
		level_editor->pos.x += 16;
		break;

	case MSS_TILE:
		level_editor->pos.x += 128;
		break;

	case MSS_LIGHT:
		level_editor->pos.x += 128;
		break;

	case MSS_ERASE:
		level_editor->pos.x += 16;
		break;
	}

	if (level_editor->pos.x > level_editor->width) level_editor->pos.x = level_editor->width;
}

void level_editor_pos_up(LevelEditor *level_editor)
{
	if (!level_editor) return NULL;

	switch (level_editor->select)
	{
	case MSS_NONE:
		level_editor->pos.y -= 16;
		break;

	case MSS_ENTITY:
		level_editor->pos.y -= 16;
		break;

	case MSS_TILE:
		level_editor->pos.y -= 128;
		break;

	case MSS_LIGHT:
		level_editor->pos.y -= 128;
		break;

	case MSS_ERASE:
		level_editor->pos.y -= 16;
		break;
	}

	if (level_editor->pos.y < 0) level_editor->pos.y = 0;
}

void level_editor_pos_down(LevelEditor *level_editor)
{
	if (!level_editor) return NULL;

	switch (level_editor->select)
	{
	case MSS_NONE:
		level_editor->pos.y += 16;
		break;

	case MSS_ENTITY:
		level_editor->pos.y += 16;
		break;

	case MSS_TILE:
		level_editor->pos.y += 128;
		break;

	case MSS_LIGHT:
		level_editor->pos.y += 128;
		break;

	case MSS_ERASE:
		level_editor->pos.y += 16;
		break;
	}

	if (level_editor->pos.y > level_editor->height) level_editor->pos.y = level_editor->height;
}

void level_editor_change_select(LevelEditor *level_editor, Uint32 select)
{
	if (!level_editor) return;
	if (!(select < MSS_NONE) && !(select > MSS_MAX - 1))
	{
		level_editor->select = select;
		
		// MSS_TILE & MSS_lIGHT are STRICT
		// They NEED their coordinates to be a multiple of 128 for level_save to work
		if (select == MSS_TILE || select == MSS_LIGHT)
		{
			// adjust x position
			int mod = (int)(level_editor->pos.x) % 128;
			if (mod != 0) level_editor->pos.x = level_editor->pos.x - mod;

			// adjust y position
			mod = (int)(level_editor->pos.y) % 128;
			level_editor->pos.y = level_editor->pos.y - mod;
		}
	}
}

void level_editor_apply_select(LevelEditor *level_editor)
{
	EntData *ent_data;

	if (!level_editor) return;

	switch (level_editor->select)
	{
	case MSS_NONE:
		return;

	case MSS_ENTITY:
		ent_data = gfc_list_get_nth(level_editor->ent_list, level_editor->ent);
		level_add_entity(level_editor->level, level_editor->pos, ent_data->name, ent_data->sprite);
		break;

	case MSS_TILE:
		level_add_tile(level_editor->level, gfc_vector2d(level_editor->pos.x/128, level_editor->pos.y/128), level_editor->tile);
		break;

	case MSS_LIGHT:
		level_add_light(level_editor->level, gfc_vector2d(level_editor->pos.x/128, level_editor->pos.y/128), level_editor->light);
		break;

	case MSS_ERASE:
		//level_delete_entity(level_editor->level, level_editor->pos);
		//level_delete_tile(level_editor->tile, level_editor->pos);
		//level_delete_light(level_editor->light, level_editor->pos);
		break;
	}

	level_editor_update_texture(level_editor);
}

void level_editor_update_texture(LevelEditor *level_editor)
{
	int i, c;
	SDL_Surface *world_surf;
	Sprite *sprite;
	GFC_Vector2D pos, scale;
	LevelLight *light;
	LevelTile *tile;
	LevelEntity *ent;

	if (!level_editor) return;

	world_surf = gf2d_graphics_create_surface(12800, 12800);

	// draw lights first
	sprite = gf2d_sprite_load_all("images/light_source.png", 128, 128, 0, 1);

	c = gfc_list_get_count(level_editor->level->lights);
	for (i = 0; i < c; i++)
	{
		light = gfc_list_get_nth(level_editor->level->lights, i);
		if (!light) continue;

		pos = gfc_vector2d(light->pos.x * 128 - 0.5 * light->rad * 128, light->pos.y * 128 - 0.5 * light->rad * 128);
		scale = gfc_vector2d(light->rad, light->rad);
		gf2d_sprite_draw_to_surface(sprite, pos, &scale, NULL, 0, world_surf);
	}

	// draw tiles second
	sprite = gf2d_sprite_load_all("images/backgrounds/tileset.png", 128, 128, 1, 1);

	c = gfc_list_get_count(level_editor->level->tiles);
	for (i = 0; i < c; i++)
	{
		tile = gfc_list_get_nth(level_editor->level->tiles, i);
		if (!tile) continue;

		pos = gfc_vector2d(tile->pos.x * 128, tile->pos.y * 128);
		gf2d_sprite_draw_to_surface(sprite, pos, NULL, NULL, tile->index, world_surf);
	}

	// draw entities last
	c = gfc_list_get_count(level_editor->level->entities);
	for (i = 0; i < c; i++)
	{
		ent = gfc_list_get_nth(level_editor->level->entities, i);
		if (!ent) continue;

		sprite = ent->sprite;
		pos = gfc_vector2d(ent->pos.x - 0.5 * ent->sprite->frame_w, ent->pos.y - 0.5 * ent->sprite->frame_h);
		gf2d_sprite_draw_to_surface(sprite, pos, NULL, NULL, 0, world_surf);
	}

	SDL_DestroyTexture(level_editor->world);
	level_editor->world = SDL_CreateTextureFromSurface(gf2d_graphics_get_renderer(), world_surf);
}

void level_editor_draw(LevelEditor *level_editor, GFC_Rect rect)
{
	SDL_Rect src, target;
	GFC_Rect cam;
	Sprite *sprite;
	GFC_Vector2D pos;

	if (!level_editor) return;

	cam = camera_get_rect();
	src.x = cam.x;
	src.y = cam.y;
	src.w = cam.w;
	src.h = cam.h;

	target.x = rect.x;
	target.y = rect.y;
	target.w = rect.w;
	target.h = rect.h;

	SDL_RenderCopy(gf2d_graphics_get_renderer(), level_editor->bg, NULL, &target);
	SDL_RenderCopy(gf2d_graphics_get_renderer(), level_editor->world, &src, &target);

	sprite = gf2d_sprite_load_all("images/pointer.png", 32, 32, 16, 0);
	pos = gfc_vector2d(175 + level_editor->pos.x + camera_get_offset().x, 160 + level_editor->pos.y + camera_get_offset().y);
	gf2d_sprite_render(sprite, pos, NULL, NULL, NULL, NULL, NULL, NULL, 2);
}

void level_editor_free(LevelEditor *level_editor)
{
	if (!level_editor) return;
	if (level_editor->level) level_free(level_editor->level);
	SDL_DestroyTexture(level_editor->bg);
	SDL_DestroyTexture(level_editor->world);
	gfc_list_foreach(level_editor->ent_list, (gfc_work_func*)free);
	gfc_list_delete(level_editor->ent_list);
	free(level_editor);
}
