#include "simple_logger.h"
#include "gfc_input.h"
#include "collision.h"
#include "camera.h"
#include "item_pickup.h"
#include "drone.h"

extern int player_focus;

/**
* @brief run the think function for the drone
*/
void drone_think(Entity* self);

/**
* @brief run the update function for the drone
*/
void drone_update(Entity* self);

/**
* @brief free the bullet
*/
void drone_free(Entity* self);

Entity* drone_new(Entity* owner)
{
	Entity* self;

	self = entity_new();
	if (!self)
	{
		slog("Failed to spawn a drone entity");
		return NULL;
	}
	self->sprite = gf2d_sprite_load_all(
		"images/drone.png",
		32,
		32,
		1,
		0);
	self->frame = 0;
	self->position = owner->position;
	self->newPosition = self->position;
	self->velocity = gfc_vector2d(0, 0);
	self->collision = gfc_vector2d(0, 0);
	self->owner = owner;
	self->think = drone_think;
	self->update = drone_update;
	self->free = drone_free;

	return self;
}

void drone_think(Entity* self)
{
	GFC_Vector2D dir;

	if (!self) return;

	dir = gfc_vector2d(0, 0);
	if (!player_focus)
	{
		if (gfc_input_command_down("up")) dir.y += -1;
		if (gfc_input_command_down("down")) dir.y += 1;
		if (gfc_input_command_down("left")) dir.x += -1;
		if (gfc_input_command_down("right")) dir.x += 1;
	}

	gfc_vector2d_normalize(&dir);
	gfc_vector2d_scale(self->velocity, dir, 4);

	// check new position for world collision
	gfc_vector2d_add(self->newPosition, self->newPosition, self->velocity);
	self->collision = collide_with_world(self->world->tileCount, self->world->physicsLayer, self->box, self->velocity);
	if (self->collision.x == 1)
	{
		self->newPosition.x = self->position.x;
	}
	if (self->collision.y == 1)
	{
		self->newPosition.y = self->position.y;
	}
}

void drone_update(Entity* self)
{
	if (!self) return;
	self->position = self->newPosition;
	if (!player_focus) camera_center_on(self->position);
}

void drone_free(Entity* self)
{
	if (!self) return;
}