#include <simple_logger.h>
#include "gfc_vector.h"
#include "bullet.h"

/**
 * @brief run the think function for the bullet
 */
void bullet_think(Entity* self);

/**
 * @brief run the update function for the bullet
 */
void bullet_update(Entity* self);

/**
 * @brief free the bullet
 */
void bullet_free(Entity* self);

Entity* bullet_new(Entity* owner, GFC_Vector2D pos)
{
	Entity* self;

	self = entity_new();
	if (!self)
	{
		slog("Failed to spawn a bullet entity");
		return NULL;
	}
	slog("new bullet");
	self->sprite = gf2d_sprite_load_all(
		"images/bullet.png",
		32,
		32,
		1,
		1);
	self->frame = 0;
	self->position = pos;
	self->range = 100;
	self->owner = owner;
	self->think = bullet_think;
	self->update = bullet_update;
	self->free = bullet_free;

	return self;
}

void bullet_think(Entity* self)
{
	if (!self) return;
}

void bullet_update(Entity* self)
{
	if (!self) return;
}

void bullet_free(Entity* self)
{
	if (!self) return;
}