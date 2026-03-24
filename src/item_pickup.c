#include "simple_logger.h"
#include "collision.h"
#include "item_pickup.h"

extern const float gravity;

/**
* @brief run the think function for the player
*/
void item_pickup_think(Entity *self);

/**
* @brief run the update function for the player
*/
void item_pickup_update(Entity *self);

/**
* @brief free the player
*/
void item_pickup_free(Entity *self);

Entity *item_pickup_new(GFC_Vector2D position, const char *name, const char *filename, int frame_w, int frame_h)
{
	Entity *self;

	self = entity_new();
	if (!self)
	{
		slog("Failed to spawn an item pickup entity");
		return NULL;
	}

	// item_pickup defaults
	gfc_line_cpy(self->name, name);
	self->layer = EL_ITEM;
	self->sprite = gf2d_sprite_load_all(filename, frame_w, frame_h, 1, 0);
	self->frame = 0;
	self->position = position;
	self->newPosition = self->position;
	self->box = gfc_rect(self->position.x, self->position.y, self->sprite->frame_w, self->sprite->frame_h);
	self->velocity = gfc_vector2d(0, 0);
	self->acceleration = gfc_vector2d(0, gravity);
	self->collision = gfc_vector2d(0, 0);
	self->think = item_pickup_think;
	self->update = item_pickup_update;
	self->free = item_pickup_free;

	return self;
}

void item_pickup_think(Entity* self)
{
	if (!self) return;

	self->velocity.y += gravity; // gravity
	if (self->velocity.y > 7) self->velocity.y = 7; // max falling speed

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
		self->velocity.y = 0;
	}
}

void item_pickup_update(Entity* self)
{
	self->position = self->newPosition;
	if (!self) return;
}

void item_pickup_free(Entity *self)
{
	if (!self) return;
}