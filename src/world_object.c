#include "simple_logger.h"
#include "collision.h"
#include "world_object.h"

extern const float gravity;

typedef enum
{
	WOTT_NONE,
	WOTT_PLAYER,
	WOTT_PROJECTILE,	
	WOTT_INPUT
} WorldObjectTriggerType;

typedef enum
{
	WOUT_SPAWN,
	WOUT_MOVE,
	WOUT_FADE,
	WOUT_DELETE
} WorldObjectUpdateType;

typedef struct
{
	GFC_TextLine	object;
	Uint32			update_type;
} WorldObjectUpdate;

typedef struct
{
	Uint8		is_static;
	Uint8		triggered;
	Uint32		trigger_type;
	GFC_List	*update_list;
} WorldObjectData;

/**
* @brief load a world object from a config file
* @param proj_name: name of the world object to load
* @return NULL on error, a world object otherwise
*/
Entity *world_object_load(const char *obj_name);

/**
* @brief run the think function for the world object
*/
void world_object_think(Entity *self);

/**
* @brief run the update function for the world object
*/
void world_object_update(Entity *self);

/**
* @brief free the world object
*/
void world_object_free(Entity *self);

Entity *world_object_load(const char *obj_name)
{
	SJson *json, *ejson, *ojson, *ujson, *array;
	const char *name, *filename, *trigger;
	float box_w, box_h;
	GFC_Rect box;
	Sint32 frame_w, frame_h, frames_per_line;
	Sprite *sprite;
	Uint8 is_static;
	Uint32 trigger_type;
	GFC_List *update_list;
	int i, c;
	WorldObjectUpdate *update;
	Entity *self;
	WorldObjectData *data;

	json = sj_load("defs/world_objects.json");
	if (!json)
	{
		slog("failed to load world object config file");
		return NULL;
	}
	ejson = sj_object_get_value(json, "world_object_entity");
	if (!ejson)
	{
		free(json);
		slog("missing world object entities object");
		return NULL;
	}
	ojson = entity_object_get_by_name(ejson, obj_name);
	if (!ojson)
	{
		free(json);
		slog("failed to find world object '%s'", obj_name);
		return NULL;
	}

	// get world object basic info
	name = sj_object_get_value_as_string(ojson, "name");
	if (!name)
	{
		free(json);
		slog("missing world object entity name");
		return NULL;
	}

	array = sj_object_get_value(ojson, "box");
	if (!array)
	{
		free(json);
		slog("missing box object for world object entity");
		return NULL;
	}
	if (sj_array_get_count(array) != 2)
	{
		free(json);
		slog("missing or extra box parameters for world object entity");
		return NULL;
	}
	if (!sj_get_float_value(sj_array_get_nth(array, 0), &box_w) ||
		!sj_get_float_value(sj_array_get_nth(array, 1), &box_h))
	{
		free(json);
		slog("one or more box parameters are invalid for world object entity");
		return NULL;
	}
	box = gfc_rect(0, 0, box_w, box_h);

	array = sj_object_get_value(ojson, "sprite");
	if (!array)
	{
		free(json);
		slog("missing sprite object for world object entity");
		return NULL;
	}
	if (sj_array_get_count(array) != 4)
	{
		free(json);
		slog("missing or extra sprite parameters for world object entity");
		return NULL;
	}
	filename = sj_get_string_value(sj_array_get_nth(array, 0));
	if (!filename ||
		!sj_get_integer_value(sj_array_get_nth(array, 1), &frame_w) ||
		!sj_get_integer_value(sj_array_get_nth(array, 2), &frame_h) ||
		!sj_get_integer_value(sj_array_get_nth(array, 3), &frames_per_line))
	{
		free(json);
		slog("one or more sprite parameters are invalid for world object entity");
		return NULL;
	}
	sprite = gf2d_sprite_load_all(filename, frame_w, frame_h, frames_per_line, 0);

	// get world object data
	if (!sj_object_get_value_as_uint8(ojson, "static", &is_static))
	{
		free(json);
		slog("missing static object for world object entity");
		return NULL;
	}

	trigger = sj_object_get_value_as_string(ojson, "trigger");
	if (!trigger)
	{
		free(json);
		slog("missing trigger object for world object entity");
		return NULL;
	}
	if (gfc_strlcmp(trigger, "none") == 0) // short-circuit (skip the updates part)
	{
		free(json);

		self = entity_new();
		if (!self)
		{
			slog("failed to spawn a world object entity");
			return NULL;
		}
		gfc_line_cpy(self->name, name);
		self->box = box;
		self->sprite = sprite;

		data = gfc_allocate_array(sizeof(WorldObjectData), 1);
		if (!data)
		{
			slog("failed to allocate world object entity data");
			return NULL;
		}
		self->data = data;
		data->is_static = is_static;
		data->trigger_type = WOTT_NONE;
		data->update_list = NULL;
		return self;
	}
	else if (gfc_strlcmp(trigger, "player") == 0) trigger_type = WOTT_PLAYER;
	else if (gfc_strlcmp(trigger, "projectile") == 0) trigger_type = WOTT_PROJECTILE;
	else if (gfc_strlcmp(trigger, "input") == 0) trigger_type = WOTT_INPUT;
	else
	{
		free(json);
		slog("invalid trigger object for world object entity");
		return NULL;
	}

	array = sj_object_get_value(ojson, "updates");
	if (!array)
	{
		free(json);
		slog("missing updates object for world object entity");
		return NULL;
	}
	update_list = gfc_list_new();
	if (!update_list)
	{
		free(json);
		slog("failed to make an update list for world object entity");
		return NULL;
	}
	c = sj_array_get_count(array);
	for (i = 0; i < c; i++)
	{
		const char *object, *type;
		Uint32 update_type;

		ujson = sj_array_get_nth(array, i);
		if (!ujson) continue;

		object = sj_object_get_value_as_string(ujson, "object");
		if (!object)
		{
			free(json);
			slog("missing object object in update #%i for world object entity", i);
			return NULL;
		}

		type = sj_object_get_value_as_string(ujson, "type");
		if (!type)
		{
			free(json);
			slog("missing type object in update #%i for world object entity", i);
			return NULL;
		}
		if (gfc_strlcmp(type, "spawn") == 0) update_type = WOUT_SPAWN;
		else if (gfc_strlcmp(type, "move") == 0) update_type = WOUT_MOVE;
		else if (gfc_strlcmp(type, "fade") == 0) update_type = WOUT_FADE;
		else if (gfc_strlcmp(type, "delete") == 0) update_type = WOUT_DELETE;
		else
		{
			free(json);
			slog("invalid update object in update #%i for world object entity", i);
			return NULL;
		}

		update = gfc_allocate_array(sizeof(WorldObjectUpdate), 1);
		if (!update)
		{
			free(json);
			slog("failed to allocate update #%i for world object entity", i);
			return NULL;
		}
		gfc_line_cpy(update->object, object);
		update->update_type = update_type;
		gfc_list_append(update_list, update);
	}
	free(json);

	// set world object basic info
	self = entity_new();
	if (!self)
	{
		slog("failed to spawn a world object entity");
		return NULL;
	}
	gfc_line_cpy(self->name, name);
	self->box = box;
	self->sprite = sprite;

	// set world object data
	data = gfc_allocate_array(sizeof(WorldObjectData), 1);
	if (!data)
	{
		slog("failed to allocate world object entity data");
		return NULL;
	}
	self->data = data;
	data->is_static = is_static;
	data->trigger_type = trigger_type;
	data->update_list = update_list;
	return self;
}

Entity *world_object_new(GFC_Vector2D position, const char *obj_name)
{
	Entity* self;
	WorldObjectData* data;

	self = world_object_load(obj_name);
	if ((!self) || (!self->data)) return NULL;
	data = self->data;

	// world object defaults
	self->layer = EL_WORLD;
	self->mask = WORLD_MASK;

	self->frame = 0;

	self->position = position;
	self->newPosition = self->position;
	if (data->is_static) self->acceleration = gfc_vector2d(0, 0);
	else self->acceleration = gfc_vector2d(0, gravity);
	self->collision = gfc_vector2d(0, 0);

	self->think = world_object_think;
	self->update = world_object_update;
	self->free = world_object_free;

	// world object data
	data->triggered = 0;

	return self;
}

void world_object_think(Entity *self)
{
	WorldObjectData* data;

	if (!self) return;
	data = self->data;

	// physics
	if (!data->is_static)
	{
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

	// trigger detection
	switch (data->trigger_type)
	{
		case WOTT_NONE:
			break;
		case WOTT_PLAYER:
			break;
		case WOTT_PROJECTILE:
			break;
		case WOTT_INPUT:
			break;
	}
}

void world_object_update(Entity *self)
{
	WorldObjectData *data;
	int i, c;
	WorldObjectUpdate *update;

	if (!self) return;
	data = self->data;

	self->position = self->newPosition;	// update position
	
	// update handling
	if (data->triggered)
	{
		c = gfc_list_get_count(data->update_list);
		for (i = 0; i < c; i++)
		{
			update = gfc_list_get_nth(data->update_list, i);
			if (!update) continue; // may slog later

			switch (update->update_type)
			{
				case WOUT_SPAWN:
					break;
				case WOUT_MOVE:
					break;
				case WOUT_FADE:
					data->triggered = 0;
					self->fade = 1;
					break;
				case WOUT_DELETE:
					break;
			}
		}
	}
}

void world_object_free(Entity *self)
{
	WorldObjectData* data;

	if ((!self) || (!self->data)) return;
	data = (WorldObjectData*) self->data;
	gfc_list_foreach(data->update_list, (gfc_work_func*)free);
	gfc_list_delete(data->update_list);
	free(data);
}

