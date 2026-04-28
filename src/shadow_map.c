#include "simple_logger.h"
#include "gf2d_draw.h"
#include "gf2d_graphics.h"
#include "camera.h"
#include "shadow_map.h"

ShadowMap *shadow_map_new(GFC_Rect dimensions)
{
	ShadowMap *shadow_map;
	SDL_Surface *surface;
	Sprite *sprite;
	GFC_Vector2D scale, offset;

	shadow_map = gfc_allocate_array(sizeof(ShadowMap), 1);
	if (!shadow_map)
	{
		slog("failed to allocate shadow map");
		return NULL;
	}
	shadow_map->shadow_layer = dimensions;

	surface = gf2d_graphics_create_surface(shadow_map->shadow_layer.w, shadow_map->shadow_layer.h);

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

	sprite = gf2d_sprite_load_all("images/light_source.png", 128, 128, 0, 1);
	scale = gfc_vector2d(7, 7);

	gf2d_sprite_draw_to_surface(
		sprite,
		gfc_vector2d(500, 750),
		&scale,
		NULL,
		0,
		surface
	);

	shadow_map->shadow_mask = SDL_CreateTextureFromSurface(gf2d_graphics_get_renderer(), surface);
	SDL_SetTextureBlendMode(shadow_map->shadow_mask, SDL_BLENDMODE_MOD);

	return shadow_map;
}

void shadow_map_free(ShadowMap *shadow_map)
{
	free(shadow_map);
}

void shadow_map_draw(ShadowMap *shadow_map)
{
	GFC_Rect rect;
	SDL_Rect sdl_rect;

	rect = camera_get_rect();
	sdl_rect.x = rect.x;
	sdl_rect.y = rect.y;
	sdl_rect.w = rect.w;
	sdl_rect.h = rect.h;
	SDL_RenderCopy(gf2d_graphics_get_renderer(), shadow_map->shadow_mask, &sdl_rect, NULL);
	//SDL_SetTextureColorMod();
}