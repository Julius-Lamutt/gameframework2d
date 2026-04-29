#include "simple_logger.h"
#include "gf2d_draw.h"
#include "gf2d_graphics.h"
#include "camera.h"
#include "shadow_map.h"

ShadowMap *shadow_map_new(Uint16 world_w, Uint16 world_h, float tile_size, Light *lights)
{
	int i, c;
	ShadowMap *shadow_map;
	Light *light;
	SDL_Surface *surface;
	Sprite *sprite;
	GFC_Vector2D pos, scale;

	if (!lights)
	{
		slog("cannot make shadow map without lights (or empty light list)");
		return NULL;
	}

	shadow_map = gfc_allocate_array(sizeof(ShadowMap), 1);
	if (!shadow_map)
	{
		slog("failed to allocate shadow map");
		return NULL;
	}

	// surface for the shadow mask
	surface = gf2d_graphics_create_surface(world_w * tile_size, world_h * tile_size);

	// draw a shadow over the entire surface
	sprite = gf2d_sprite_load_all("images/darkness.png", 128, 128, 0, 1);
	scale = gfc_vector2d(30, 12);

	gf2d_sprite_draw_to_surface(
		sprite,
		gfc_vector2d(0, 0),
		&scale,
		NULL,
		0,
		surface
	);

	// draw lights on top of the shadow
	sprite = gf2d_sprite_load_all("images/light_source.png", tile_size, tile_size, 0, 1);

	c = gfc_list_get_count(lights);
	for (i = 0; i < c; i++)
	{
		light = gfc_list_get_nth(lights, i);
		if (!light) continue;
		pos = gfc_vector2d(light->pos.x - 0.5 * light->r1 * tile_size, light->pos.y - 0.5 * light->r2 * tile_size);
		scale = gfc_vector2d(light->r1, light->r2);

		gf2d_sprite_draw_to_surface(
			sprite,
			pos,
			&scale,
			NULL,
			0,
			surface
		);
	}

	shadow_map->shadow_mask = SDL_CreateTextureFromSurface(gf2d_graphics_get_renderer(), surface);
	SDL_SetTextureBlendMode(shadow_map->shadow_mask, SDL_BLENDMODE_MOD);

	return shadow_map;
}

void shadow_map_draw(ShadowMap *shadow_map)
{
	if (!shadow_map) return NULL;

	GFC_Rect rect;
	SDL_Rect sdl_rect;

	rect = camera_get_rect();
	sdl_rect.x = rect.x;
	sdl_rect.y = rect.y;
	sdl_rect.w = rect.w;
	sdl_rect.h = rect.h;
	SDL_RenderCopy(gf2d_graphics_get_renderer(), shadow_map->shadow_mask, &sdl_rect, NULL);
}

void shadow_map_free(ShadowMap *shadow_map)
{
	if (!shadow_map) return;

	gfc_list_foreach(shadow_map->lights, (gfc_work_func*)light_free);
	gfc_list_delete(shadow_map->lights);
	free(shadow_map);
}