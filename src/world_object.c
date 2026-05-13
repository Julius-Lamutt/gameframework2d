#include "simple_logger.h"
#include "gfc_input.h"
#include "shadow_map.h"
#include "physics.h"
#include "ai.h"
#include "world_object.h"

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
	WOUT_LIGHT,
	WOUT_DELETE
} WorldObjectUpdateType;

typedef struct
{
	Uint32			update_type;
	GFC_TextLine	object;
} WorldObjectUpdate;

typedef struct
{
	World		*world;
	Uint8		is_static;
	Uint8		triggered;
	Uint32		trigger_type;
	GFC_List	*update_list;
	Entity		*linked_ent;
	Entity      *linked_ent2;
	Uint8		moving;
	float		move_speed;
	Uint32		move_time;
	Uint16		move_dis;
	Uint8		light_on;
	Uint8		force_trigger;
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

/**
* @brief get the world object's touch updates for this frame
* @param self: the world object to get touch updates for
*/
void world_object_get_touch_updates(Entity *self);

Entity *world_object_load(const char *obj_name)
{
	SJson *json, *ejson, *ojson, *ujson, *array;
	const char *name, *filename, *trigger;
	float box_w, box_h, fall_speed;
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
	sprite = gf2d_sprite_load_all(filename, frame_w, frame_h, frames_per_line, 1);

	if (!sj_object_get_value_as_float(ojson, "fall_speed", &fall_speed))
	{
		free(json);
		slog("missing fall_speed object for player entity");
		return NULL;
	}

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
		self->fall_speed = fall_speed;

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
			slog("missing new_object object in update #%i for world object entity", i);
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
		else if (gfc_strlcmp(type, "light") == 0) update_type = WOUT_LIGHT;
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
	self->fall_speed = fall_speed;

	// set world object data
	data = gfc_allocate_array(sizeof(WorldObjectData), 1);
	if (!data)
	{
		entity_free(self);
		slog("failed to allocate world object entity data");
		return NULL;
	}
	self->data = data;
	data->is_static = is_static;
	data->trigger_type = trigger_type;
	data->update_list = update_list;
	return self;
}

Entity *world_object_new(World *world, GFC_Vector2D position, const char *obj_name)
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

	self->think = world_object_think;
	self->update = world_object_update;
	self->free = world_object_free;

	// world object data
	data->world = world;
	data->triggered = 0;
	data->linked_ent = NULL;
	data->linked_ent2 = NULL;

	// special data for rope
	if (gfc_strlcmp(self->name, "object_rope") == 0)
	{
		data->linked_ent = world_object_new(data->world, gfc_vector2d(self->position.x, self->position.y + 60), "object_light");
	}
	// special data for elevator
	else if (gfc_strlcmp(self->name, "object_elevator") == 0)
	{
		self->fade = 1;
		data->moving = 0;
		data->move_speed = -2;
		data->move_time = 0;
		data->move_dis = 0;
		data->linked_ent = world_object_new(data->world, gfc_vector2d(self->position.x, self->position.y - 64), "object_elevator_floor");
		data->linked_ent2 = world_object_new(data->world, gfc_vector2d(self->position.x, self->position.y + 64), "object_elevator_floor");
	}
	// special data for light
	else if (gfc_strlcmp(self->name, "object_lamp") == 0)
	{
		self->layer = EL_NONE;
		data->moving = 0;
		data->light_on = 1;
		data->force_trigger = 0;
		data->linked_ent = light_new(gfc_vector2d(self->position.x, self->position.y - 20), 2, 2);
		if (world)
		{
			shadow_map_add_light(world->shadowMap, data->linked_ent);
		}
		else slog("missing world for light world object creation");

		ai_add_light_id(self->id);
	}
	return self;
}

void world_object_think(Entity *self)
{
	WorldObjectData* data;

	if (!self) return;
	data = self->data;

	world_object_get_touch_updates(self);

	// update physics for non-static world objects
	if (!data->is_static)
	{
		// get current velocity, then get new position
		physics_get_velocity(self->box, &self->velocity, &self->fall_speed);
		gfc_vector2d_add(self->newPosition, self->newPosition, self->velocity);
	}

	// elevator physics
	if (data->moving)
	{
		self->velocity = gfc_vector2d(0, data->move_speed);
		gfc_vector2d_add(self->newPosition, self->newPosition, self->velocity);

		data->linked_ent->velocity = gfc_vector2d(0, data->move_speed);
		gfc_vector2d_add(data->linked_ent->newPosition, data->linked_ent->newPosition, data->linked_ent->velocity);

		data->linked_ent2->velocity = gfc_vector2d(0, data->move_speed);
		gfc_vector2d_add(data->linked_ent2->newPosition, data->linked_ent2->newPosition, data->linked_ent2->velocity);

		data->move_dis += abs(data->move_speed);

		if (data->move_dis >= 768)
		{
			data->moving = 0;
			data->move_speed *= -1;
			data->move_dis = 0;
			self->velocity = gfc_vector2d(0, 0);
			data->linked_ent->velocity = gfc_vector2d(0, 0);
			data->linked_ent2->velocity = gfc_vector2d(0, 0);
		}
	}
}

void world_object_update(Entity *self)
{
	int i, c;
	WorldObjectData *data;
	WorldObjectUpdate *update;

	if (!self) return;
	data = self->data;

	self->position = self->newPosition;	// update position
	
	// world object update handling
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
					if (gfc_strlcmp(self->name, "object_rope") == 0)
					{
						world_object_new(data->world, gfc_vector2d(self->position.x, self->position.y + 60), update->object);
					}
					else world_object_new(data->world, self->position, update->object);
					break;

				case WOUT_MOVE:
					data->moving = 1;
					data->move_time = SDL_GetTicks();
					break;

				case WOUT_FADE:
					self->fade = 1;
					break;

				case WOUT_LIGHT:
					if (!data->linked_ent)
					{
						data->linked_ent = light_new(gfc_vector2d(self->position.x, self->position.y - 20), 2, 2);
						if (data->world)
						{
							shadow_map_add_light(data->world->shadowMap, data->linked_ent);
						}
						else slog("missing world for light world object creation");
					}
					else
					{
						if (data->world)
						{
							shadow_map_remove_light(data->world->shadowMap, data->linked_ent);
						}
						else slog("missing world for light world object creation");
						data->linked_ent = NULL;
					}

					if (data->light_on) data->light_on = 0;
					else data->light_on = 1;

					break;

				case WOUT_DELETE:
					if (gfc_strlcmp(self->name, "object_rope") == 0)
					{
						entity_free(data->linked_ent);
					}
					else entity_free(self);
					break;
			}
		}
	}
	else
	{
		// set defaults when there is no updates
		if (gfc_strlcmp(self->name, "object_elevator") != 0) self->fade = 0;
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

void world_object_get_touch_updates(Entity *self)
{
	int i, c;
	Entity *other;
	WorldObjectData *data;
	GFC_Vector2D collision;

	if (!self || !self->data) return;
	data = (WorldObjectData*) self->data;
	data->triggered = 0; // reset trigger

	entity_get_entity_touches(self); // get entities touched this frame

	c = gfc_list_get_count(self->entity_touches);
	for (i = 0; i < c; i++)
	{
		other = gfc_list_get_nth(self->entity_touches, i);
		if (!other) continue;
		if (other->layer == EL_PLAYER)
		{
			if (data->trigger_type == WOTT_PLAYER) data->triggered = 1;
			if (data->trigger_type == WOTT_INPUT && !data->moving)
			{
				if (gfc_input_key_pressed("e")) data->triggered = 1;
			}
		}
		else if (other->layer == EL_PROJECTILE)
		{
			if (data->trigger_type == WOTT_PROJECTILE)
			{
				if (gfc_strlcmp(other->name, "projectile_shuriken") == 0) data->triggered = 1;
			}
		}
	}

	// handle force trigger
	if (gfc_strlcmp(self->name, "object_lamp") == 0)
	{
		if (data->force_trigger)
		{
			data->force_trigger = 0;
			data->triggered = 1;
		}
	}
}

Uint8 *world_object_light_on(Entity *self)
{
	WorldObjectData *data;

	if (!self || !self->data) return 2;
	data = (WorldObjectData*) self->data;

	if (gfc_strlcmp(self->name, "object_lamp") != 0) return 2;
	return data->light_on;
}

void world_object_light_trigger(Entity *self)
{
	WorldObjectData *data;

	if (!self || !self->data) return;
	data = (WorldObjectData*) self->data;

	if (gfc_strlcmp(self->name, "object_lamp") != 0) return;
	data->force_trigger = 1;
}