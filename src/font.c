#include <SDL.h>
#include <SDL_ttf.h>
#include "simple_logger.h"
#include "gfc_list.h"

typedef enum
{
	FS_SMALL,
	FS_MEDIUM,
	FS_LARGE,
	FS_MAX
} FontStyles;

typedef struct
{
	GFC_List *fonts;
} FontManager;

static FontManager _font_manager = {0};

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
	if (!_font_manager.fonts)
	{
		slog("failed to create list of fonts");
		return;
	}
	for (i = 0; i < FS_MAX; i++)
	{
		font = TTF_OpenFont("fonts/PixelifySans/PixelifySans-Regular.tff", 10 + (i * 4));
	}
	if (!font)
	{
		slog("failed to open font file");
		return;
	}
	gfc_list_append(_font_manager.fonts, font);
	atexit(font_close);
}

void font_close()
{
	int i, c;
	TTF_Font *font;

	// other cleanup first
	c = gfc_list_get_count(_font_manager.fonts);
	for (i = 0; i < c; i++)
	{
		font = gfc_list_get_nth(_font_manager.fonts, i);
		if (!font) continue;
		TTF_CloseFont(font);
	}
	gfc_list_delete(_font_manager.fonts);
	memset(&_font_manager, 0, sizeof(FontManager));
	TTF_Quit();
}