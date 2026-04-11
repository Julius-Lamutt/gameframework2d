#include "simple_logger.h"
#include "collision.h"
#include "item_pickup.h"

extern const float gravity;

/**
* @brief load an item pickup from a config file
* @param item_name: the name of the item pickup to load
* @return NULL on error, an item pickup otherwise
*/
Entity *item_pickup_load(const char *item_name);

/**
* @brief run the think function for the item pickup
*/
void item_pickup_think(Entity *self);

/**
* @brief run the update function for the item pickup
*/
void item_pickup_update(Entity *self);

/**
* @brief free the item pickup
*/
void item_pickup_free(Entity *self);

Entity *item_pickup_load(const char *item_name)
{
	SJson *json, *ejson, *ijson, *array;
	int i, c;
	const char *name, *filename;
	float box_w, box_h;
	GFC_Rect box;
	Sint32 frame_w, frame_h, frames_per_line;
	Sprite *sprite;
	Entity *self;

	json = sj_load("defs/item_pickups.json");
	if (!json)
	{
		slog("failed to load item pickup config file");
		return NULL;
	}
	ejson = sj_object_get_value(json, "item_pickup_entity");
	if (!ejson)
	{
		free(json);
		slog("missing item pickup entities object");
		return NULL;
	}

	c = sj_array_get_count(ejson);
	for (i = 0; i < c; i++)
	{
		const char *name;

		ijson = sj_array_get_nth(ejson, i);
		if (!ijson) continue;
		name = sj_object_get_value_as_string(ijson, "name");
		if (!name)
		{
			free(json);
			slog("missing item pickup entity name");
			return NULL;
		}
		if (gfc_strlcmp(name, item_name) == 0) break;
	}

	if (i == c)
	{
		free(json);
		slog("failed to find pickup item '%s'", item_name);
		return NULL;
	}
	ijson = sj_array_get_nth(ejson, i);

	name = sj_object_get_value_as_string(ijson, "name");
	if (!name)
	{
		free(json);
		slog("missing item pickup entity name");
		return NULL;
	}

	array = sj_object_get_value(ijson, "box");
	if (!array)
	{
		free(json);
		slog("missing box object for item pickup entity");
		return NULL;
	}
	if (sj_array_get_count(array) != 2)
	{
		free(json);
		slog("missing or extra box parameters for item pickup entity");
		return NULL;
	}
	if (!sj_get_float_value(sj_array_get_nth(array, 0), &box_w) ||
		!sj_get_float_value(sj_array_get_nth(array, 1), &box_h))
	{
		free(json);
		slog("one or more box parameters are invalid for item pickup entity");
		return NULL;
	}
	box = gfc_rect(0, 0, box_w, box_h);

	array = sj_object_get_value(ijson, "sprite");
	if (!array)
	{
		free(json);
		slog("missing sprite object for item pickup entity");
		return NULL;
	}
	if (sj_array_get_count(array) != 4)
	{
		free(json);
		slog("missing or extra sprite parameters for item pickup entity");
		return NULL;
	}
	filename = sj_get_string_value(sj_array_get_nth(array, 0));
	if (!filename ||
		!sj_get_integer_value(sj_array_get_nth(array, 1), &frame_w) ||
		!sj_get_integer_value(sj_array_get_nth(array, 2), &frame_h) ||
		!sj_get_integer_value(sj_array_get_nth(array, 3), &frames_per_line))
	{
		free(json);
		slog("one or more sprite parameters are invalid for item pickup entity");
		return NULL;
	}
	sprite = gf2d_sprite_load_all(filename, frame_w, frame_h, frames_per_line, 0);

	free(json);

	self = entity_new();
	if (!self)
	{
		slog("failed to spawn an item pickup entity");
		return NULL;
	}
	gfc_line_cpy(self->name, name);
	self->box = box;
	self->sprite = sprite;
	return self;
}

Entity *item_pickup_new(GFC_Vector2D position, const char *item_name)
{
	Entity *self;

	self = item_pickup_load(item_name);
	if (!self) return NULL;

	// item_pickup defaults
	self->layer = EL_ITEM;
	self->mask = ITEM_MASK;

	self->frame = 0;

	self->position = position;
	self->newPosition = self->position;
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