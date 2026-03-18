#ifndef __FONT_H__
#define __FONT_H__

#include "gfc_text.h"
#include "gfc_color.h"

typedef enum
{
	FS_SMALL,
	FS_MEDIUM,
	FS_LARGE,
	FS_MAX
} FontStyles;

/*
* @brief initialize the font system
*/
void font_init();

/*
* @brief text font rendering and wrapping
* @param text: the text to be rendered
* @param style: the font style to render the text in
* @param color: the color to render the text in
* @param position: where to render the text
*/
void font_draw_text(const char *text, FontStyles style, GFC_Color color, GFC_Vector2D position);

/*
* @brief add a recently used text render to the cache
* @param text: the text that was rendered
* @param style: the font style the text was rendered in
* @param size: the size of the texture used to render the text
* @param color: the color the text was rendered in
* @param texture: the texture that was used to render the text
*/
void font_add_recent(const char *text, FontStyles style, GFC_Vector2D size, GFC_Color color, SDL_Texture *texture);

/*
* @brief call this function every frame to cleanup the font cache
*/
void font_cleanup();

#endif
