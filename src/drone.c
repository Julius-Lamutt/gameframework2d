#include "simple_logger.h"
#include "gfc_input.h"
#include "physics.h"
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
	self->layer = EL_PROJECTILE;
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

	// drone movement
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

	// get current velocity, then get new position
	float fall_speed = 1;
	physics_get_velocity(self->box, &self->velocity, &fall_speed);
	gfc_vector2d_add(self->newPosition, self->newPosition, self->velocity);
}

void drone_update(Entity* self)
{
	if (!self) return;

	// update physics
	self->position = self->newPosition;

	// center camera on drone if it is being used
	if (!player_focus) camera_center_on(self->position);
}

void drone_free(Entity* self)
{
	if (!self) return;
}