#ifndef __ELEMENT_LABEL_H__
#define __ELEMENT_LABEL_H__

#include "simple_json.h"
#include "elements.h"

typedef struct
{
	GFC_TextBlock	text;	/* the text for the label */
	int				style;  /* the style for the label */
	GFC_Color		color;	/* the color for the label */
	int				wrap;	/* if true wrap at element width */
} LabelElement;

/*
* @brief load an label element
* @param windel: the window element to load the label from
* @return NULL on error, a pointer to an element label otherwise
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
* @param bounds: the bounds for the label
*/
void element_label_draw(LabelElement *label, GFC_Rect bounds);

#endif
