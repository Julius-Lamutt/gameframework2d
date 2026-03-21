#ifndef __ELEMENT_LABEL_H__
#define __ELEMENT_LABEL_H__

#include "font.h"
#include "elements.h"

typedef struct
{
	GFC_TextBlock	text;	/* the text for the label */
	FontStyles		style;  /* the style for the label */
	GFC_Color		color;	/* the color for the label */
	int				wrap;	/* if true wrap at element width */
} LabelElement;

/*
* @brief load a label element
* @param windel: the window element to load the label element from
* @return NULL on error, a pointer to an label element otherwise
*/
LabelElement *element_label_load(SJson *windel);

/*
* @brief free a label element from memory
* @param label: the label element to be freed
*/
void element_label_free(LabelElement *label);

/*
* @brief draws a label element
* @param label: the label element to draw
* @param bounds: the bounds for the label element
*/
void element_label_draw(LabelElement *label, GFC_Rect bounds);

#endif
