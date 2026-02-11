#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "gfc_shape.h"

typedef struct
{
	GFC_Rect bounds;

} Camera;

GFC_Vector2D camera_get_postion();

GFC_Vector2D camera_get_offset();



#endif
