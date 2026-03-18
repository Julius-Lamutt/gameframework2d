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
	ET_LIST,
	ET_LABEL,
	ET_ACTOR,
	ET_BUTTON,
	ET_ENTRY
} ElementTypes;

typedef struct Element_S
{

} Element;

#endif
