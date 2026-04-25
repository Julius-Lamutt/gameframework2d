#include "simple_logger.h"
#include "collision.h"
#include "camera.h"

Bool collide_with_entity(GFC_Rect self_box, GFC_Vector2D self_velocity, GFC_Rect other_box, GFC_Vector2D other_velocity)
{
	GFC_Rect self_rect;
	GFC_Rect other_rect;

	gfc_rect_set(self_rect, self_box.x + self_velocity.x, self_box.y + self_velocity.y, self_box.w, self_box.h);
	gfc_rect_set(other_rect, other_box.x + other_velocity.x, other_box.y + other_velocity.y, other_box.w, other_box.h);

	if (gfc_rect_overlap(self_rect, other_rect)) return true;
	return false;
}

GFC_Vector2D collide_with_entity_vector(GFC_Rect self_box, GFC_Vector2D self_velocity, GFC_Rect other_box, GFC_Vector2D other_velocity)
{
	int test_both, test_x, test_y;
	GFC_Rect self_rect, self_rect_x, self_rect_y, other_rect;

	test_both = 0;
	test_x = 0;
	test_y = 0;

	gfc_rect_set(self_rect, self_box.x + self_velocity.x, self_box.y + self_velocity.y, self_box.w, self_box.h);
	gfc_rect_set(self_rect_x, self_box.x + self_velocity.x, self_box.y, self_box.w, self_box.h);
	gfc_rect_set(self_rect_y, self_box.x, self_box.y + self_velocity.y, self_box.w, self_box.h);
	gfc_rect_set(other_rect, other_box.x + other_velocity.x, other_box.y + other_velocity.y, other_box.w, other_box.h);

	if (!test_both && gfc_rect_overlap(self_rect, other_rect)) test_both = 1;
	if (!test_x && gfc_rect_overlap(self_rect_x, other_rect)) test_x = 1;
	if (!test_y && gfc_rect_overlap(self_rect_y, other_rect)) test_y = 1;
	
	if (test_both)
	{
		if (test_x && !test_y) return gfc_vector2d(1, 0);
		if (test_y && !test_x) return gfc_vector2d(0, 1);
		return gfc_vector2d(1, 1);
	}
	return gfc_vector2d(0, 0);
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

		if (!test_both && gfc_rect_overlap(box_test, tile)) test_both = 1;
		if (!test_x && gfc_rect_overlap(box_test_x, tile)) test_x = 1;
		if (!test_y && gfc_rect_overlap(box_test_y, tile)) test_y = 1;
	}
	if (test_both)
	{
		if (test_x && !test_y) return gfc_vector2d(1, 0);
		if (test_y && !test_x) return gfc_vector2d(0, 1);
		return gfc_vector2d(1, 1);
	}
	return gfc_vector2d(0, 0);
}

Uint8 collide_with_world_floor_or_ceiling(Uint32 tile_count, GFC_Rect* physics_layer, GFC_Rect box)
{
	int i;
	float epsilon = 3;
	GFC_Rect box_test, box_test_y, tile;
	GFC_Vector2D offset;

	gfc_rect_set(box_test, box.x - (epsilon/2), box.y - (epsilon/2), box.w + epsilon, box.h + epsilon);
	gfc_rect_set(box_test_y, box.x, box.y, box.w, box.h);

	offset = camera_get_offset();

	for (i = 0; i < tile_count; i++)
	{
		tile.x = physics_layer[i].x + offset.x;
		tile.y = physics_layer[i].y + offset.y;
		tile.w = physics_layer[i].w;
		tile.h = physics_layer[i].h;

		if (gfc_rect_overlap(box_test, tile))
		{
			if (box_test.y < tile.y + tile.h && box_test.y > tile.y) return 0; // ceiling
			if (box_test.y + box_test.h > tile.y && box_test.y + box_test.h < tile.y + tile.h) return 1; // floor
		}
		if (gfc_rect_overlap(box_test_y, tile))
		{
			if (box_test_y.y < tile.y + tile.h && box_test_y.y > tile.y) return 0; // ceiling
			if (box_test_y.y + box_test_y.h > tile.y && box_test_y.y + box_test_y.h < tile.y + tile.h) return 1; // floor
		}
	}
	return 2; // falling
}

Uint8 collide_with_object_floor_or_ceiling(GFC_Rect self_box, GFC_Rect other_box)
{
	float epsilon = 5;
	GFC_Rect self_rect, self_rect_y, other_rect, other_rect_y;

	gfc_rect_set(self_rect, self_box.x - (epsilon/2), self_box.y - (epsilon/2), self_box.w + epsilon, self_box.h + epsilon);
	gfc_rect_set(self_rect_y, self_box.x, self_box.y, self_box.w, self_box.h);

	gfc_rect_set(other_rect, other_box.x, other_box.y, other_box.w, other_box.h);
	gfc_rect_set(self_rect_y, other_box.x, other_box.y, other_box.w, other_box.h);

	if (gfc_rect_overlap(self_rect, other_rect))
	{
		if (self_rect.y < other_rect.y + other_rect.h && self_rect.y > other_rect.y) return 0; // ceiling
		if (self_rect.y + self_rect.h > other_rect.y && self_rect.y + self_rect.h < other_rect.y + other_rect.h) return 1; // floor
	}
	return 2; // falling
}
