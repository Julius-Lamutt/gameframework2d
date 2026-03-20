#include <SDL_ttf.h>
#include "simple_logger.h"
#include "gfc_list.h"
#include "gf2d_graphics.h"
#include "font.h"

typedef struct
{
	char *text;
	SDL_Texture *texture;
	FontStyles style;
	GFC_Vector2D size;
	GFC_Color color;
	Uint32 timestamp;
} FontCache;

typedef struct
{
	GFC_List *fonts;
	GFC_List *recents;
} FontManager;

static FontManager _font_manager = {0};

/*
* @brief closes the font system
*/
void font_close();

/*
* @brief retrieve a specific font cache if it exists
* @param text: the text of the cache
* @param style: the font style of the cache
* @param color: the text color of the cache
* @return NULL if cache is not found, a font cache otherwise
*/
FontCache *font_get_recent(const char *text, FontStyles style, GFC_Color color);

/*
* @brief free a font cache and its data
* @param cache: the cache to free
*/
void font_cache_free(FontCache *cache);

void font_init()
{
	int i;
	TTF_Font *font;

	if (TTF_Init() != 0)
	{
		slog("failed to initialize TTF system");
		return;
	}
	_font_manager.fonts = gfc_list_new();
	_font_manager.recents = gfc_list_new();
	if (!_font_manager.fonts)
	{
		slog("failed to create list of fonts");
		return;
	}
	for (i = 0; i < FS_MAX; i++)
	{
		font = TTF_OpenFont("fonts/Pixelify_Sans/PixelifySans-Regular.ttf", 20 + (i * 8));
		if (!font)
		{
			slog("failed to open font file");
			return;
		}
		gfc_list_append(_font_manager.fonts, font);
	}
	atexit(font_close);
	slog("fonts initialized");
}

void font_close()
{
	int i, c;
	TTF_Font *font;
	FontCache *cache;

	// other cleanup first
	c = gfc_list_get_count(_font_manager.fonts);
	for (i = 0; i < c; i++)
	{
		font = gfc_list_get_nth(_font_manager.fonts, i);
		if (!font) continue;
		TTF_CloseFont(font);
	}
	gfc_list_delete(_font_manager.fonts);
	c = gfc_list_get_count(_font_manager.recents);
	for (i = 0; i < c; i++)
	{
		cache = gfc_list_get_nth(_font_manager.recents, i);
		if (!cache) continue;
		font_cache_free(cache);
	}
	gfc_list_delete(_font_manager.recents);
	memset(&_font_manager, 0, sizeof(FontManager));
	TTF_Quit();
	slog("fonts closed");
}

void font_draw_text(const char *text, FontStyles style, GFC_Color color, GFC_Vector2D position, Uint32 wrap_length)
{
	TTF_Font *font;
	SDL_Surface *surface;
	SDL_Texture *texture;
	SDL_Rect rect;
	SDL_Color fg;
	FontCache *cache;

	cache = font_get_recent(text, style, color);
	if (cache)
	{
		rect.x = position.x;
		rect.y = position.y;
		rect.w = cache->size.x;
		rect.h = cache->size.y;
		SDL_RenderCopy(gf2d_graphics_get_renderer(), cache->texture, NULL, &rect);
		cache->timestamp = SDL_GetTicks();
		return;
	}
	font = gfc_list_get_nth(_font_manager.fonts, style);
	if (!font)
	{
		slog("failed to render text '%s', missing font style %i", text, style);
		return;
	}
	fg = gfc_color_to_sdl(color);
	surface = TTF_RenderUTF8_Blended_Wrapped(font, text, fg, wrap_length);
	if (!surface)
	{
		slog("failed to render text '%s', failed to create surface", text);
		return;
	}
	surface = gf2d_graphics_screen_convert(&surface);
	if (!surface)
	{
		slog("failed to render text '%s', failed to create surface", text);
		return;
	}
	texture = SDL_CreateTextureFromSurface(gf2d_graphics_get_renderer(), surface);
	if (!texture)
	{
		SDL_FreeSurface(surface);
		slog("failed to render text '%s', failed to create texture", text);
		return;
	}
	rect.x = position.x;
	rect.y = position.y;
	rect.w = surface->w;
	rect.h = surface->h;
	SDL_RenderCopy(gf2d_graphics_get_renderer(), texture, NULL, &rect);
	SDL_FreeSurface(surface);
	font_add_recent(text, style, gfc_vector2d(rect.w, rect.h), color, texture);
}

void font_add_recent(const char *text, FontStyles style, GFC_Vector2D size, GFC_Color color, SDL_Texture *texture)
{
	FontCache *cache;
	size_t length;

	if ((!text) || (!texture)) return;
	cache = gfc_allocate_array(sizeof(FontCache), 1);
	if (!cache) return;
	length = strlen(text) + 1;
	cache->text = gfc_allocate_array(sizeof(char), length);
	strncpy(cache->text, text, length);
	cache->style = style;
	gfc_vector2d_copy(cache->size, size);
	gfc_color_copy(cache->color, color);
	cache->texture = texture;
	cache->timestamp = SDL_GetTicks();
	gfc_list_append(_font_manager.recents, cache);
}

FontCache *font_get_recent(const char *text, FontStyles style, GFC_Color color)
{
	int i, c;
	FontCache *cache;

	if (!text) return NULL;
	c = gfc_list_get_count(_font_manager.recents);
	for (i = 0; i < c; i++)
	{
		cache = gfc_list_get_nth(_font_manager.recents, i);
		if (!cache) continue;
		if (gfc_strlcmp(cache->text, text) != 0) continue;
		if (cache->style != style) continue;
		if (!gfc_color_cmp(cache->color, color)) continue;
		return cache;
	}
	return NULL;
}

void font_cache_free(FontCache *cache)
{
	if (!cache) return;
	if (cache->text) free(cache->text);
	if (cache->texture) SDL_DestroyTexture(cache->texture);
	free(cache);
}

void font_cleanup()
{
	int i, c;
	Uint32 now;
	FontCache *cache;

	now = SDL_GetTicks();
	c = gfc_list_get_count(_font_manager.recents);
	for (i = c - 1; i >= 0; i--)
	{
		cache = gfc_list_get_nth(_font_manager.recents, i);
		if (!cache) continue;
		if (now > cache->timestamp + 1000)
		{
			font_cache_free(cache);
			gfc_list_delete_nth(_font_manager.recents, i);
		}
	} 
}