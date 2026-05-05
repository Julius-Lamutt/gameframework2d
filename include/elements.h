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
	ET_LABEL,	/* text blocks */
	ET_ACTOR,	/* sprites/animations */
	ET_BUTTON,	/* triggers an action on click */
	ET_ENTRY	/* allows user keyboard inputs */
} ElementTypes;

typedef struct Element_S
{
	GFC_TextLine	name;		/* name of element, should be unique */
	Uint32			type;		/* element type e.g. label, actor */
	Uint32			state;		/* element state e.g. disable, highlight */
	Uint8			updated;	/* if true then element will be acted on by parent window */
	Uint8			can_focus;	/* true if element can be the focus of keyboard input */
	Uint8			has_focus;	/* true if element has the focus of keyboard input */
	GFC_Rect		bounds;		/* drawing bounds for element */
	GFC_Color		color;		/* color for the element */

	void *data; // used for specific element data determined by type
} Element;

/*
* @brief load all window elements given a list of elements
* @param element_list: the json list of window elements
* @param win: the parent window for the list of elements
* @return NULL on error, a list of window elements otherwise
*/
GFC_List *element_list_load(SJson *element_list, Window *win);

/*
* @brief draw elements from a list
* @param element_list: the list of elements to draw
*/
void element_list_draw(GFC_List *element_list);

/*
* @brief free a list of elements from memory
* @param element_list: the list of elements to be freed
*/
void element_list_free(GFC_List *element_list);

/*
* @brief update an element state based on its type
* @param element: the element to update the state for
*/
void element_update_state(Element *element);

/*
* @brief update the text for a label
* @param element: the element to update
* @param text: the new text for the label
*/
void element_update_label(Element *element, const char *text);

/*
* @brief update the sprite for an actor
* @param sprite: the new sprite for the actor (NULL to keep the same sprite)
* @param frame: which frame should be used for the actor
*/
void element_update_actor(Element *element, Sprite *sprite, Uint8 frame);

#endif
