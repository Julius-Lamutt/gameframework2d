#ifndef __INTERACTABLES_H__
#define __INTERACTABLES_H__

#include "entity.h"

/**
* @brief spawn an interactable
* @param name: position for interctable
* @param name: name of pickup
* @param filename: the file for the interactable
* @param frame_w: frame width
* @param frame_h: frame height
* return NULL on error, a pointer to the interactable otherwise
**/
Entity *interactable_new(GFC_Vector2D position, const char *name, const char *filename, int frame_w, int frame_h);

#endif
