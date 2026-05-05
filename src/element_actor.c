#include "simple_logger.h"
#include "element_actor.h"

ActorElement *element_actor_load(SJson *windel)
{
	ActorElement *actor;
	SJson *array;
	Sprite *image;
	const char *filename;
	int frame_w, frame_h, frames_per_line;
	GFC_Color color;

	if (!windel)
	{
		slog("failed to find window element");
		return NULL;
	}
	actor = gfc_allocate_array(sizeof(ActorElement), 1);
	if (!actor)
	{
		slog("failed to allocate actor element");
		return NULL;
	}

	array = sj_object_get_value(windel, "image");
	if (!array)
	{
		free(actor);
		slog("failed to find image object for actor element");
		return NULL;
	}
	if (sj_array_get_count(array) != 4)
	{
		free(actor);
		slog("missing image parameters for actor element");
		return NULL;
	}
	filename = sj_get_string_value(sj_array_get_nth(array, 0));
	if (!filename ||
		!sj_get_integer_value(sj_array_get_nth(array, 1), &frame_w) ||
		!sj_get_integer_value(sj_array_get_nth(array, 2), &frame_h) ||
		!sj_get_integer_value(sj_array_get_nth(array, 3), &frames_per_line))
	{
		free(actor);
		slog("one or more image parameters are invalid for actor element");
		return NULL;
	}
	if (gfc_strlcmp(filename, "images/backgrounds/tileset.png") == 0)
	{
		image = gf2d_sprite_load_all(filename, frame_w, frame_h, frames_per_line, 1);
	}
	else image = gf2d_sprite_load_all(filename, frame_w, frame_h, frames_per_line, 0);

	array = sj_object_get_value(windel, "color_shift");
	if (sj_array_get_count(array) != 4)
	{
		free(actor);
		slog("missing or extra color parameters for actor element");
		return NULL;
	}
	if (!sj_get_float_value(sj_array_get_nth(array, 0), &color.r) ||
		!sj_get_float_value(sj_array_get_nth(array, 1), &color.g) ||
		!sj_get_float_value(sj_array_get_nth(array, 2), &color.b) ||
		!sj_get_float_value(sj_array_get_nth(array, 3), &color.a))
	{
		free(actor);
		slog("one or more color parameters are invalid for actor element");
		return NULL;
	}
	color.ct = CT_RGBA8;

	actor->image = image;
	actor->color_shift = color;
	actor->frame = 0;
	return actor;
}

void element_actor_draw(ActorElement *actor, GFC_Rect bounds)
{
	GFC_Vector2D position, scale;
	GFC_Color color_shift;

	if (!actor) return;
	position = gfc_vector2d(bounds.x, bounds.y);
	scale = gfc_vector2d(bounds.w / actor->image->frame_w, bounds.h / actor->image->frame_h);
	color_shift = actor->color_shift;

	gf2d_sprite_draw(actor->image, 
		position, // draw background within window size
		&scale, // scale image to match element bounds
		NULL,
		NULL,
		NULL,
		&color_shift,
		actor->frame);
}

void element_actor_free(ActorElement *actor)
{
	if (!actor) return;
	gf2d_sprite_free(actor->image);
	free(actor);
}