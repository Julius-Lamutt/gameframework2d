#include "simple_logger.h"
#include "gfc_shape.h"
#include "collision.h"

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

Bool collide_with_world(Entity *self, World *world)
{
	int i;
	GFC_Rect	self_box;

	if (!self || !world)
	{
		slog("missing entity or world for collision test");
		return false;
	}
	if (!world->physicsLayer)
	{
		slog("missing physics layer for world collision test");
		return false;
	}

	self_box = (gfc_rect(self->newPosition.x - (0.5 * self->sprite->frame_w), self->newPosition.y - (0.5 * self->sprite->frame_h),
		self->sprite->frame_w, self->sprite->frame_h));
	for (i = 0; i < world->tileCount; i++)
	{
		if (((self_box.x + self_box.w >= world->physicsLayer[i].x) &&
			(self_box.x <= world->physicsLayer[i].x + world->physicsLayer[i].w)) &&
			((self_box.y + self_box.h >= world->physicsLayer[i].y) &&
			(self_box.y <= world->physicsLayer[i].y + world->physicsLayer[i].h))) return true;
	}
	return false;
}
