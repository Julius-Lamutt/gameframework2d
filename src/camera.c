#include "simple_logger.h"
#include "gfc_vector.h"
#include "camera.h";

static Camera camera = {0};

GFC_Vector2D camera_get_position()
{
	return; // gfc_vector2d(camera.x, camera.y);
}

GFC_Vector2D camera_get_offset()
{
	return gfc_vector2d(-camera.bounds.x, -camera.bounds.y);
}


