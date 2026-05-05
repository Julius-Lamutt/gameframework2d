#ifndef __ELEMENT_ENTRY_H__
#define __ELEMENT_ENTRY_H__

#include "elements.h"
#include "element_label.h"

typedef struct {
	GFC_TextLine	buffer;		/* buffer used to store user input */
	LabelElement	*output;	/* user input will be outputted to screen using label element */
} EntryElement;

/*
* @brief load entry element
* @param windel: the window element to load the entry element from
* @return NULL on error, a pointer to an entry element otherwise
*/
EntryElement *element_entry_load(SJson *windel);

/*
* @brief draws an entry element
* @param button: the entry element to draw
* @param element: a pointer to basic element data
*/
void element_entry_draw(EntryElement *entry, Element *element);

/*
* @brief free an entry element from memory
* @param label: the entry element to be freed
*/
void element_entry_free(EntryElement *entry);

#endif
