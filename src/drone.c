#include "simple_logger.h"
#include "drone.h"

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

Entity* bullet_new(Entity* owner, GFC_Vector2D pos)
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
	self->position = pos;
	self->owner = owner;
	self->think = drone_think;
	self->update = drone_update;
	self->free = drone_free;

	return self;
}

void drone_think(Entity* self)
{
	if (!self) return;
}

void drone_update(Entity* self)
{
	if (!self) return;
}

void drone_free(Entity* self)
{
	if (!self) return;
}