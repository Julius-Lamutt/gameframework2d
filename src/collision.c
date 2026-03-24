#include "simple_logger.h"
#include "gfc_shape.h"
#include "collision.h"
#include "camera.h"

Bool *collide_with_entity(Entity *self, Entity *other)
{
	GFC_Rect self_box;
	GFC_Rect other_box;

	if (!self || !other)
	{
		slog("missing one or more entities for collision test");
		return false;
	}

	gfc_rect_set(self_box, self->box.x + self->velocity.x, self->box.y + self->velocity.y, self->box.w, self->box.h);
	gfc_rect_set(other_box, other->box.x + other->velocity.x, other->box.y + other->velocity.y, other->box.w, other->box.h);

	if (gfc_rect_overlap(self_box, other_box)) return true;
	return false;
}

GFC_Vector2D collide_with_world(Uint32 tile_count, GFC_Rect *physics_layer, GFC_Rect box, GFC_Vector2D velocity)
{
	int i, test_both, test_x, test_y;
	GFC_Rect box_test, box_test_x, box_test_y, tile;
	GFC_Vector2D offset;

	test_both = 0;
	test_x = 0;
	test_y = 0;
	
	gfc_rect_set(box_test, box.x + velocity.x, box.y + velocity.y, box.w, box.h);
	gfc_rect_set(box_test_x, box.x + velocity.x, box.y, box.w, box.h);
	gfc_rect_set(box_test_y, box.x, box.y + velocity.y, box.w, box.h);

	offset = camera_get_offset();

	for (i = 0; i < tile_count; i++)
	{
		tile.x = physics_layer[i].x + offset.x;
		tile.y = physics_layer[i].y + offset.y;
		tile.w = physics_layer[i].w;
		tile.h = physics_layer[i].h;

		if (!test_both)
		{
			if (gfc_rect_overlap(box_test, tile))
			{
				test_both = 1;
			}
		}

		if (!test_x)
		{
			if (gfc_rect_overlap(box_test_x, tile))
			{
				test_x = 1;
			}
		}

		if (!test_y)
		{
			if (gfc_rect_overlap(box_test_y, tile))
			{
				test_y = 1;
			}
		}
	}
	if (test_both)
	{
		if (test_x && !test_y) return gfc_vector2d(1, 0);
		if (test_y && !test_x) return gfc_vector2d(0, 1);
		return gfc_vector2d(1, 1);
	}
	return gfc_vector2d(0, 0);
}
