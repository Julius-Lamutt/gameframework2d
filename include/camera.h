#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "gfc_shape.h"

/*
* @brief get the offset to draw objects relative to the camera
* @return the offset
*/
GFC_Vector2D camera_get_offset();

/*
* @brief get the camera's position in world space
* @return the position
*/
GFC_Vector2D camera_get_position();

/*
* @brief get an rectangle that represents the camera's vision
* @return a rectangle
*/
GFC_Rect camera_get_rect();

/*
* @brief move camera to the given position
* @param position: where to move the camera
*/
void camera_set_position(GFC_Vector2D position);

/*
* @brief center the camera on a certain position
* @param target: where to center the camera at
*/
void camera_center_on(GFC_Vector2D target);

/*
* @brief set the size of the camera
* @param size: the new camera size
*/
void camera_set_size(GFC_Vector2D size);

/*
* @brief set the camera's bounds
* @param bounds: the new camera bounds
*/
void camera_set_bounds(GFC_Rect bounds);

/*
* @brief snap the camera into world bounds
*/
void camera_apply_bounds();

/*
* @brief enable/disable camera binding to world bounds
* @param bindCamera: enable/disable camera binding
*/
void camera_enable_binding(Bool bindCamera);

#endif