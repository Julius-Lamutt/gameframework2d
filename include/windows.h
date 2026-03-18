#ifndef __WINDOWS_H__
#define __WINDOWS_H__

#include <SDL.h>
#include "gfc_list.h"
#include "gfc_shape.h"
#include "gfc_text.h"
#include "gf2d_sprite.h"

typedef struct Window_S
{
	Uint8			_inuse;			/* no touchy */
	Uint8			hidden;			/* no drawing or updating if true */
	GFC_TextLine	name;			/* window name, should be unique */
	GFC_List		*elements;		/* list of elements that should be updated */
	Sprite			*background;	/* window background */
	Sprite			*border;		/* window border */
	GFC_Rect		dim;			/* where on the screen to draw window */
	GFC_Rect		canvas;			/* where on the window to draw elements */
	struct Window_S *parent;		/* pointer to parent window */
	struct Window_S *child;			/* pointer to child window */
	Uint8			block_input		/* block input checks for child windows, but allows update */
	void (*close_child)(struct Window_S *win, struct Window_S *child);
	int (*update)(struct Window_S *win, GFC_List *updateElements);
	int (*draw)(struct Window_S *win);
	int (*free)(struct Window_S *win);
	void *data;
} Window;

/**
* @brief this initializes the window management system and queues up cleaning on exit
* @param max: the maximum number of windows that can exist at the same time
*/
void window_system_init(Uint32 max);

#endif
