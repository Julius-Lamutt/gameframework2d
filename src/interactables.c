#include "simple_logger.h"
#include "interactables.h"
#include "actor.h"
#include "collision.h"

extern const float gravity;

/**
* @brief run the think function for the interactable
*/
void interactable_think(Entity *self);

/**
* @brief run the update function for the interactable
*/
void interactable_update(Entity *self);

/**
* @brief free the interactable
*/
void interactable_free(Entity *self);

Entity *interactable_new(GFC_Vector2D position, const char *name, const char *filename, int frame_w, int frame_h)
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
	self->layer = EL_WORLD;
	self->sprite = gf2d_sprite_load_all(filename, frame_w, frame_h, 1, 0);
	self->frame = 0;
	self->position = position;
	self->newPosition = self->position;
	self->box = gfc_rect(self->position.x, self->position.y, self->sprite->frame_w, self->sprite->frame_h);
	self->velocity = gfc_vector2d(0, 0);
	self->acceleration = gfc_vector2d(0, 0);
	self->collision = gfc_vector2d(0, 0);
	self->proj = NULL;
	if (gfc_strlcmp(self->name, "rope") == 0)
	{
		self->proj = interactable_new(gfc_vector2d(self->position.x, self->position.y + 32), "light", "images/light.png", 32, 32);
	}
	self->think = interactable_think;
	self->update = interactable_update;
	self->free = interactable_free;

	return self;
}

void interactable_think(Entity* self)
{
	if (!self) return;
}

void interactable_update(Entity* self)
{
	if (!self) return;
}

void interactable_free(Entity *self)
{
	if (!self) return;
	if (self->proj)
	{
		actor_new(self->proj->position, "actor_light", "images/light.png", 32, 32);
		entity_free(self->proj);
	}
}