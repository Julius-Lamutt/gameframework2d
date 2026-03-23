#include "simple_logger.h"
#include "physics.h"
#include "collision.h"
#include "item_shuriken.h"

extern const float gravity;

/**
* @brief run the think function for the shuriken
*/
void shuriken_think(Entity* self);

/**
* @brief run the update function for the shuriken
*/
void shuriken_update(Entity* self);

/**
* @brief free the shuriken
*/
void shuriken_free(Entity* self);

typedef struct
{
	float distance;
} ShurikenData;

Entity *shuriken_new(Entity *owner, GFC_Vector2D dir)
{
	Entity *self;
	ShurikenData *data;

	self = entity_new();
	if (!self)
	{
		slog("Failed to spawn a shuriken entity");
		return NULL;
	}

	// shuriken defaults
	gfc_line_cpy(self->name, "Shuriken");
	self->layer = EL_PROJECTILE;
	self->sprite = gf2d_sprite_load_all("images/shuriken.png", 16, 16, 1, 0);
	self->frame = 0;
	self->position = owner->position;
	gfc_vector2d_add(self->position, self->position, gfc_vector2d(dir.x * 30, dir.y * 30));
	self->newPosition = self->position;
	self->box = gfc_rect(self->position.x, self->position.y, self->sprite->frame_w, self->sprite->frame_h);
	gfc_vector2d_scale(self->velocity, dir, 12);
	self->acceleration = gfc_vector2d(0, gravity);
	self->collision = gfc_vector2d(0, 0);
	self->owner = owner;
	self->proj = NULL;
	self->think = shuriken_think;
	self->update = shuriken_update;
	self->free = shuriken_free;
	self->range = 1000;

	// client data
	data = gfc_allocate_array(sizeof(ShurikenData), 1);
	if (data) self->data = data;
	return self;
}

void shuriken_think(Entity* self)
{
	ShurikenData *data;

	if (!self) return;

	data = self->data; // get shuriken data

	if (self->velocity.x != 0) self->velocity.y += gravity; // gravity
	if (self->velocity.y > 4) self->velocity.y = 4; // max falling speed

	// check new position for world collision
	gfc_vector2d_add(self->newPosition, self->newPosition, self->velocity);
	self->collision = collide_with_world(self->box, self->velocity, self->world);
	if (self->collision.x == 1)
	{
		self->newPosition.x = self->position.x;
		self->velocity = gfc_vector2d(0, 0);
		data->distance = 0;
	}
	if (self->collision.y == 1)
	{
		self->newPosition.y = self->position.y;
		self->velocity = gfc_vector2d(0, 0);
		data->distance = 0;
	}
	if (self->collision.x == 0 && self->collision.y == 0)
	{
		data->distance += gfc_vector2d_magnitude(self->velocity);
	}
}

void shuriken_update(Entity* self)
{
	int i, c;
	Entity *other;
	ShurikenData* data;

	if (!self) return;

	data = self->data; // get shuriken data

	self->position = self->newPosition;
	if (data->distance >= self->range) entity_free(self);
}

void shuriken_free(Entity *self)
{
	ShurikenData* data;

	if ((!self) || (!self->data)) return;
	data = (ShurikenData*) self->data;
	free(data);
}