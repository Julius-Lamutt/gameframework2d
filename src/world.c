#include "simple_logger.h"
#include "simple_json.h"
#include "gfc_shape.h"
#include "gf2d_draw.h"
#include "gf2d_graphics.h"
#include "collision.h"
#include "item_pickup.h"
#include "player.h"
#include "camera.h"
#include "world.h"

extern const Bool f_collision_draw;

/**
* @brief build the tile layer for the world
* @param world: the world to build tile layer on
*/
void world_build_tile_layer(World *world);

/**
* @brief build the physics layer for the world
* @param world: the world to build physics layer on
*/
void world_build_physics_layer(World *world);

/*
* @brief load pre-cached entities for the world
* @param world: the world to load the entities in
* @param ejson: the json array of entities to load
*/
void world_entity_load(World *world, SJson *ejson);

/**
* @brief draw the physics layer
* @param world: which world's physics layer to draw
* @param offset: how far the physics layer should be drawn from the origin
*/
void world_draw_physics_layer(World *world, GFC_Vector2D offset);

void world_build_tile_layer(World* world)
{
	int i, j;
	Uint32 frame, index, tile_count = 0;
	GFC_Vector2D position;

	if (!world) return;
	if (!world->tileSet) return;
	if (world->tileLayer)
	{
		gf2d_sprite_free(world->tileLayer);
	}
	world->tileLayer = gf2d_sprite_new();

	world->tileLayer->surface = gf2d_graphics_create_surface(
		world->tileWidth * world->tileSet->frame_w, 
		world->tileHeight * world->tileSet->frame_h
	);

	world->tileLayer->frame_w = world->tileWidth * world->tileSet->frame_w;
	world->tileLayer->frame_h = world->tileHeight * world->tileSet->frame_h;

	if (!world->tileLayer->surface)
	{
		slog("failed to create tile layer surface");
		return;
	}

	for (j = 0; j < world->tileHeight; j++)
	{
		for (i = 0; i < world->tileWidth; i++)
		{
			index = i + (j * world->tileWidth);
			if (world->tileMap[index] == 0) continue;
			tile_count++; // increment number of solid tiles
			position = gfc_vector2d(i * world->tileSet->frame_w, j * world->tileSet->frame_h);
			frame = world->tileMap[index] - 1;

			gf2d_sprite_draw_to_surface(
				world->tileSet,
				position,
				NULL,
				NULL,
				frame,
				world->tileLayer->surface
			);
		}
	}

	world->tileLayer->texture = SDL_CreateTextureFromSurface(gf2d_graphics_get_renderer(), world->tileLayer->surface);
	if (!world->tileLayer->texture)
	{
		slog("failed to convert world tile layer to texture");
		return;
	}
	world->tileCount = tile_count;
	world_build_physics_layer(world); // build physics layer once tile layer is complete
}

void world_build_physics_layer(World *world)
{
	int i, j;
	Uint32 t_index, p_index = 0;
	GFC_Vector2D position;
	GFC_Rect rect;

	if (!world) return;
	if (!world->tileCount) return; // may slog later
	if (!world->tileSet) return;
	if (!world->tileLayer) return;
	if (world->physicsLayer) free(world->physicsLayer);

	world->physicsLayer = gfc_allocate_array(sizeof(GFC_Rect), world->tileCount);
	if (!world->physicsLayer)
	{
		slog("failed to allocate a physics layer");
		return;
	}
	for (j = 0; j < world->tileHeight; j++)
	{
		for (i = 0; i < world->tileWidth; i++)
		{
			t_index = i + (j * world->tileWidth);
			if (world->tileMap[t_index] == 0) continue;
			position = gfc_vector2d(i * world->tileSet->frame_w, j * world->tileSet->frame_h);
			rect = gfc_rect(position.x, position.y, world->tileSet->frame_w, world->tileSet->frame_h);
			world->physicsLayer[p_index] = rect;
			p_index++;
		}
	}
}

void world_entity_load(World *world, SJson *ejson)
{
	int i, c;
	SJson *object, *array;
	float position_x, position_y;
	GFC_Vector2D position;
	Entity *entity;
	GFC_Vector2D collision;

	if (!world) return;
	if (!ejson) return;
	if (!world->physicsLayer)
	{
		slog("missing world physics layer for item layer creation");
		return;
	}
	
	c = sj_array_get_count(ejson);
	for (i = 0; i < c; i++)
	{
		const char *name;

		object = sj_array_get_nth(ejson, i);
		if (!object) continue;

		name = sj_object_get_value_as_string(object, "name");
		if (!name)
		{
			slog("missing name object for entity #%i in entities list", i);
			continue;
		}

		array = sj_object_get_value(object, "location");
		if (!array)
		{
			slog("missing location object for entity #%i in entities list", i);
			continue;
		}
		if (sj_array_get_count(array) != 2)
		{
			slog("missing location object for entity #%i in entities list", i);
			continue;
		}
		if (!sj_get_float_value(sj_array_get_nth(array, 0), &position_x) ||
			!sj_get_float_value(sj_array_get_nth(array, 1), &position_y))
		{
			slog("one or more location parameters for entity #%i in entities list are invalid", i);
			continue;
		}
		position = gfc_vector2d(position_x, position_y);

		if (gfc_strlcmp(name, "player") == 0) entity = player_new(position);
		else
		{
			slog("entity #%i in entities list has invalid entity name", i);
			continue;
		}
		//entity->world = world;
	}
	return;
}

World *world_load(const char *filename)
{
	int i, j, tile, frame_w, frame_h, frames_per_line, w = 0, h = 0;
	const char *background, *tileSet;
	World *world = NULL;
	SJson *json, *wjson, *ejson, *vertical, *horizontal, *item;

	if (!filename)
	{
		slog("no filename provided for world_load");
		return NULL;
	}
	json = sj_load(filename);
	if (!json)
	{
		slog("failed to load world file %s", filename);
		return NULL;
	}
	wjson = sj_object_get_value(json, "world");
	if (!wjson)
	{
		slog("%s missing world object", filename);
		sj_free(json);
		return NULL;
	}
	vertical = sj_object_get_value(wjson, "tileMap");
	if (!vertical)
	{
		slog("%s missing tileMap object", filename);
		sj_free(json);
		return NULL;
	}
	ejson = sj_object_get_value(json, "entities");
	if (!ejson)
	{
		sj_free(json);
		slog("missing entity list for world");
		return NULL;
	}

	h = sj_array_get_count(vertical);
	horizontal = sj_array_get_nth(vertical, 0);
	w = sj_array_get_count(horizontal);
	world = world_new(w, h);
	if (!world)
	{
		slog("failed to create world for file %s", filename);
		sj_free(json);
		return NULL;
	}
	for (j = 0; j < h; j++)
	{
		horizontal = sj_array_get_nth(vertical, j);
		if (!horizontal) continue; // may slog later, just skip for now
		for (i = 0; i < w; i++)
		{
			item = sj_array_get_nth(horizontal, i);
			if (!item) continue;
			sj_get_integer_value(item, &tile);
			world->tileMap[i + (j * w)] = tile;
		}
	}

	background = sj_object_get_value_as_string(wjson, "background");
	world->background = gf2d_sprite_load_image(background);

	tileSet = sj_object_get_value_as_string(wjson, "tileSet");
	sj_object_get_value_as_int(wjson, "frame_w", &frame_w);
	sj_object_get_value_as_int(wjson, "frame_h", &frame_h);
	sj_object_get_value_as_int(wjson, "frames_per_line", &frames_per_line);
	world->tileSet = gf2d_sprite_load_all(
		tileSet,
		frame_w,
		frame_h,
		frames_per_line,
		1
	);

	world_build_tile_layer(world);
	entity_system_set_world(world);
	world_entity_load(world, ejson);
	sj_free(json);
	return world;
}

World *world_new(Uint32 width, Uint32 height)
{
	World *world;

	if ((!width) || (!height))
	{
		slog("cannot make a world with zero width or height");
		return NULL;
	}

	world = gfc_allocate_array(sizeof(World), 1);
	if (!world)
	{
		slog("failed to allocate a new world");
		return NULL;
	}
	// all boilerplate code here
	world->tileMap = gfc_allocate_array(sizeof(Uint8), width * height);
	world->tileWidth = width;
	world->tileHeight = height;
	return world;
}

void world_free(World *world)
{
	if (!world) return;
	gf2d_sprite_free(world->background);
	gf2d_sprite_free(world->tileSet);
	gf2d_sprite_free(world->tileLayer);
	free(world->tileMap);
	free(world->physicsLayer);
	free(world);
}

void world_draw_physics_layer(World *world, GFC_Vector2D offset)
{
	int i;
	GFC_Rect rect;

	if (!world) return;
	for (i = 0; i < world->tileCount; i++)
	{
		rect.x = world->physicsLayer[i].x + offset.x;
		rect.y = world->physicsLayer[i].y + offset.y;
		rect.w = world->physicsLayer[i].w;
		rect.h = world->physicsLayer[i].h;
		gf2d_draw_rect(rect, GFC_COLOR_MAGENTA);
	}
}

void world_draw(World *world)
{
	GFC_Vector2D offset;

	if (!world) return;
	offset = camera_get_offset();
	gf2d_sprite_draw_image(world->background, gfc_vector2d(0,0));
	gf2d_sprite_draw_image(world->tileLayer, offset);
	if (f_collision_draw) world_draw_physics_layer(world, offset);
}

void world_setup_camera(World *world)
{
	if (!world) return;
	if (!world->tileLayer || !world->tileLayer->surface)
	{
		slog("no tile layer set for world");
		return;
	}
	camera_set_bounds(gfc_rect(0, 0, world->tileLayer->surface->w, world->tileLayer->surface->h));
	camera_apply_bounds();
	camera_enable_binding(true);
}