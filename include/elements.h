#ifndef __ELEMENTS_H__
#define __ELEMENTS_H__

#include <SDL.h>
#include "gfc_shape.h"
#include "gfc_text.h"
#include "gf2d_draw.h"
#include "gf2d_sprite.h"
#include "windows.h"

typedef enum
{
	ES_IDLE,		/* normal state */
	ES_DISABLE,		/* greyed out */
	ES_HIGHLIGHT,	/* cursor on element */
	ES_HIDDEN,		/* invisible to the user */
	ES_ACTIVE		/* selected */
} ElementState;

typedef enum
{
	EUR_NONE,		/* updated */
	EUR_UPDATED		/* not updated */
} ElementUpdatedReturn;

typedef enum
{
	ET_LABEL,	/* text blocks */
	ET_ACTOR,	/* sprites/animations */
	ET_BUTTON,	/* triggers an action on click */
	ET_ENTRY,	/* allows user keyboard inputs */
	ET_LIST,	/* a list of elements */
} ElementTypes;

typedef struct Element_S
{
	GFC_TextLine	name;		/* name of element, should be unique */
	int				index;		/* order of highlights, -1 to disable highlights */
	int				type;		/* element type e.g. label, actor */
	int				state;		/* element state e.g. disable, highlight */
	int				can_focus;	/* true if element can be the focus of keyboard input */
	int				has_focus;	/* true if element has the focus of keyboard input */
	GFC_Rect		bounds;		/* drawing bounds for element */
	GFC_Color		color;		/* color for the element */
	Window			*win;		/* parent window */

	void *data;
} Element;

/*
* @brief load all window elements given a list of elements
* @param element_list: the json list of window elements
* @param win: the parent window for the list of elements
* @return NULL on error, a list of window elements otherwise
*/
GFC_List *element_list_load(SJson *element_list, Window *win);

/*
* @brief free a list of elements from memory
* @param element_list: the list of elements to be freed
*/
void element_list_free(GFC_List *element_list);

/*
* @brief draw elements from a list
* @param element_list: the list of elements to draw
*/
void element_list_draw(GFC_List *element_list);

#endif
