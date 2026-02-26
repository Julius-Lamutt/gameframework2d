#include "simple_json.h"
#include "simple_logger.h"
#include "gf2d_graphics.h"
#include "world.h"

void world_build_tile_layer(World* world)
{
	int i, j;
	Uint32 frame, index;
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
			position.x = i * world->tileSet->frame_w;
			position.y = j * world->tileSet->frame_h;
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
}

World* world_test_new()
{
	int i;
	int width = 65, height = 45;
	World *world;

	world = world_new(width, height);

	if (!world) return NULL;
	world->background = gf2d_sprite_load_image("images/backgrounds/bg_flat.png");
	world->tileSet = gf2d_sprite_load_all(
		"images/backgrounds/tileset.png",
		16,
		16,
		1,
		1
	);
	for (i = 0; i < width; i++)
	{
		world->tileMap[i] = 1;
		world->tileMap[i + ((height - 1) * width)] = 1;
	}
	for (i = 0; i < height; i++)
	{
		world->tileMap[i * width] = 1;
		world->tileMap[i * width + (width - 1)] = 1;
	}
	world_build_tile_layer(world);
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
	// all defaults
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
	free(world);
}

void world_draw(World *world)
{
	if (!world) return;
	gf2d_sprite_draw_image(world->background, gfc_vector2d(0,0));
	gf2d_sprite_draw_image(world->tileLayer, gfc_vector2d(0,0));
}