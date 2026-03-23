#include "simple_logger.h"
#include "gfc_shape.h"
#include "collision.h"
#include "camera.h"

Bool collide_with_entity(Entity *self, Entity *other)
{
	GFC_Rect self_box;
	GFC_Rect other_box;

	if (!self || !other)
	{
		slog("missing one or more entities for collision test");
		return false;
	}

	self_box = (gfc_rect(self->newPosition.x - (0.5 * self->sprite->frame_w), self->newPosition.y - (0.5 * self->sprite->frame_h),
		self->sprite->frame_w, self->sprite->frame_h));
	other_box = (gfc_rect(other->newPosition.x - (0.5 * other->sprite->frame_w), other->newPosition.y - (0.5 * other->sprite->frame_h),
		other->sprite->frame_w, other->sprite->frame_h));

	if (((self_box.x + self_box.w >= other_box.x) &&
		(self_box.x <= other_box.x + other_box.w)) &&
		((self_box.y + self_box.h >= other_box.y) &&
		(self_box.y <= other_box.y + other_box.h))) return true;
	return false;
}

GFC_Vector2D collide_with_world(GFC_Rect box, GFC_Vector2D velocity, World *world)
{
	int i, test_both, test_x, test_y;
	GFC_Rect box_test, box_test_x, box_test_y, tile;
	GFC_Vector2D offset;

	if (!world)
	{
		slog("missing world for world collision test");
		return gfc_vector2d(0, 0);
	}
	if (!world->physicsLayer)
	{
		slog("missing physics layer for world collision test");
		return gfc_vector2d(0, 0);
	}

	test_both = 0;
	test_x = 0;
	test_y = 0;
	
	gfc_rect_set(box_test, box.x + velocity.x, box.y + velocity.y, box.w, box.h);
	gfc_rect_set(box_test_x, box.x + velocity.x, box.y, box.w, box.h);
	gfc_rect_set(box_test_y, box.x, box.y + velocity.y, box.w, box.h);

	offset = camera_get_offset();

	for (i = 0; i < world->tileCount; i++)
	{
		tile.x = world->physicsLayer[i].x + offset.x;
		tile.y = world->physicsLayer[i].y + offset.y;
		tile.w = world->physicsLayer[i].w;
		tile.h = world->physicsLayer[i].h;

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
