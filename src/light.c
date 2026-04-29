#include "simple_logger.h"
#include "light.h"

Light *light_new(GFC_Vector2D pos, float r1, float r2)
{
	Light *light;

	light = gfc_allocate_array(sizeof(Light), 1);
	if (!light)
	{
		slog("failed to allocate light for shadow map");
		return NULL;
	}
	light->pos = pos;
	light->r1 = r1;
	light->r2 = r2;
	return light;
}

void light_free(Light *light)
{
	free(light);
}

