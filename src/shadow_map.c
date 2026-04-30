#include "simple_logger.h"
#include "gf2d_draw.h"
#include "gf2d_graphics.h"
#include "camera.h"
#include "shadow_map.h"

static void shadow_map_update_texture(ShadowMap* shadow_map);

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
	sprite = gf2d_sprite_load_all("images/darkness.png", tile_size, tile_size, 0, 1);
	scale = gfc_vector2d(world_w, world_h);

	gf2d_sprite_draw_to_surface(
		sprite,
		gfc_vector2d(0, 0),
		&scale,
		NULL,
		0,
		surface
	);

	// draw lights on top of the shadow
	sprite = gf2d_sprite_load_all("images/light_source.png", 128, 128, 0, 1);

	c = gfc_list_get_count(lights);
	for (i = 0; i < c; i++)
	{
		light = gfc_list_get_nth(lights, i);
		if (!light) continue;
		pos = gfc_vector2d(light->pos.x - 0.5 * light->r1 * 128, light->pos.y - 0.5 * light->r2 * 128);
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

	// set defaults
	shadow_map->world_w = world_w;
	shadow_map->world_h = world_h;
	shadow_map->tile_size = tile_size;
	shadow_map->lights = lights;
	return shadow_map;
}

void shadow_map_add_light(ShadowMap *shadow_map, Light *light)
{
	if (!shadow_map || !shadow_map->lights) return;
	if (!light) return;

	gfc_list_append(shadow_map->lights, light);
	shadow_map_update_texture(shadow_map);
}

void shadow_map_remove_light(ShadowMap *shadow_map, Light *light)
{
	int i, c;
	Light *other;

	if (!shadow_map || !shadow_map->lights) return;
	if (!light) return;

	c = gfc_list_get_count(shadow_map->lights);
	for (i = 0; i < c; i++)
	{
		other = gfc_list_get_nth(shadow_map->lights, i);
		if (!other) continue;
		if (light == other)
		{
			light_free(light);
			gfc_list_delete_nth(shadow_map->lights, i);
		}
	}
	shadow_map_update_texture(shadow_map);
}

static void shadow_map_update_texture(ShadowMap *shadow_map)
{
	int i, c;
	Light *light;
	SDL_Surface *surface;
	Sprite *sprite;
	GFC_Vector2D pos, scale;

	if (!shadow_map || !shadow_map->lights) return;

	surface = gf2d_graphics_create_surface(shadow_map->world_w * shadow_map->tile_size, shadow_map->world_h * shadow_map->tile_size);

	// draw a shadow over the entire surface
	sprite = gf2d_sprite_load_all("images/darkness.png", shadow_map->tile_size, shadow_map->tile_size, 0, 1);
	scale = gfc_vector2d(shadow_map->world_w, shadow_map->world_h);

	gf2d_sprite_draw_to_surface(
		sprite,
		gfc_vector2d(0, 0),
		&scale,
		NULL,
		0,
		surface
	);

	// draw lights on top of the shadow
	sprite = gf2d_sprite_load_all("images/light_source.png", 128, 128, 0, 1);

	c = gfc_list_get_count(shadow_map->lights);
	for (i = 0; i < c; i++)
	{
		light = gfc_list_get_nth(shadow_map->lights, i);
		if (!light) continue;
		pos = gfc_vector2d(light->pos.x - 0.5 * light->r1 * 128, light->pos.y - 0.5 * light->r2 * 128);
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

	SDL_DestroyTexture(shadow_map->shadow_mask);
	shadow_map->shadow_mask = SDL_CreateTextureFromSurface(gf2d_graphics_get_renderer(), surface);
	SDL_SetTextureBlendMode(shadow_map->shadow_mask, SDL_BLENDMODE_MOD);
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

	SDL_DestroyTexture(shadow_map->shadow_mask);
	gfc_list_foreach(shadow_map->lights, (gfc_work_func*)light_free);
	gfc_list_delete(shadow_map->lights);
	free(shadow_map);
}