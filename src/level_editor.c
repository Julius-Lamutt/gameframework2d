#include "simple_logger.h"
#include "simple_json.h"
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
		sprite = gf2d_sprite_load_all(filename, frame_w, frame_h, frames_per_line, 0);

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

	level_editor = gfc_allocate_array(sizeof(LevelEditor), 1);
	if (!level_editor)
	{
		slog("failed to allocate level editor data");
		return NULL;
	}

	level_editor->ent_list = level_editor_ent_list_load();
	if (!level_editor->ent_list)
	{
		slog("failed to allocate ent data for level editor");
		return NULL;
	}

	level_editor->select = MSS_NONE;
	level_editor->width = 1280;
	level_editor->height = 1280;
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
}

void level_editor_decrease_width(LevelEditor *level_editor)
{
	if (!level_editor) return;
	if (!(level_editor->width <= 1280)) level_editor->width -= 128;
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
}

void level_editor_decrease_height(LevelEditor *level_editor)
{
	if (!level_editor) return;
	if (!(level_editor->height <= 1280)) level_editor->height -= 128;
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

void level_editor_change_select(LevelEditor *level_editor, Uint32 select)
{
	if (!level_editor) return;
	if (!(select < MSS_NONE) && !(select > MSS_MAX - 1)) level_editor->select = select;
	return;
	
	switch (select)
	{
		case MSS_NONE:
			break;

		case MSS_ENTITY:
			break;

		case MSS_TILE:
			break;

		case MSS_LIGHT:
			break;

		case MSS_ERASE:
			break;
	}
}

void level_editor_free(LevelEditor *level_editor)
{
	if (!level_editor) return;
	gfc_list_foreach(level_editor->ent_list, (gfc_work_func*)free);
	gfc_list_delete(level_editor->ent_list);
	free(level_editor);
}
