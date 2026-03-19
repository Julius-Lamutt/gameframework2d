#ifndef __ELEMENTS_H__
#define __ELEMENTS_H__

#include <SDL.h>
#include "gfc_shape.h"
#include "gfc_text.h"
#include "gf2d_sprite.h"

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
	EUR_NONE,		/* don't update */
	EUR_UPDATED		/* update */
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
	Uint8			can_focus;	/* true if element can be the focus of keyboard input */
	Uint8			has_focus;	/* true if element has the focus of keyboard input */
	GFC_Rect		bounds;		/* drawing bounds for element */
	GFC_Color		color;		/* color for the element */
} Element;

/*
* @brief load a window element
* @param element: the window element to load
* @return NULL on error, a pointer to an element otherwise
*/
Element *element_load(SJson *windel);

/*
* @brief load all window elements given a list of elements
* @param element_list: the json list of window elements
* @return NULL on error, a list of window elements otherwise
*/
GFC_List *element_list_load(SJson *element_list);

#endif
