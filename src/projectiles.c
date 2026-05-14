#include "simple_logger.h"
#include "physics.h"
#include "collision.h"
#include "windows.h"
#include "objectives_menu.h"
#include "item_pickup.h"
#include "projectiles.h"

extern const float gravity;
int destroy_stalagmite = 0;
int destroy_rope = 0;

typedef enum
{
	PCT_FALL,
	PCT_STICK,
	PCT_BOUNCE
} ProjectileContactType;

typedef struct
{
	Uint32 contact_type;
	Uint8 made_contact;
	Uint8 proj_kill;
	float distance;
} ProjectileData;

/**
* @brief load a projectile from a config file
* @param proj_name: name of the projectile to load
* @return NULL on error, a world object otherwise
*/
Entity *projectile_load(const char *proj_name);

/**
* @brief run the think function for the projectile
*/
void projectile_think(Entity *self);

/**
* @brief run the update function for the projectile
*/
void projectile_update(Entity *self);

/**
* @brief free the projectile
*/
void projectile_free(Entity *self);

/**
* @brief get the projectiles's touch updates for this frame
* @param self: the projectile to get touch updates for
*/
void projectile_get_touch_updates(Entity *self);

Entity *projectile_load(const char *proj_name)
{
	SJson *json, *ejson, *pjson, *array;
	int i, c;
	const char *name, *filename;
	float box_w, box_h, fall_speed;
	GFC_Rect box;
	Sint32 frame_w, frame_h, frames_per_line;
	Sprite *sprite;
	Entity *self;

	json = sj_load("defs/projectiles.json");
	if (!json)
	{
		slog("failed to load projectile config file");
		return NULL;
	}
	ejson = sj_object_get_value(json, "projectile_entity");
	if (!ejson)
	{
		free(json);
		slog("missing projectile entites object");
		return NULL;
	}

	c = sj_array_get_count(ejson);
	for (i = 0; i < c; i++)
	{
		const char *name;

		pjson = sj_array_get_nth(ejson, i);
		if (!pjson) continue;
		name = sj_object_get_value_as_string(pjson, "name");
		if (!name)
		{
			free(json);
			slog("missing projectile entity name");
			return NULL;
		}
		if (gfc_strlcmp(name, proj_name) == 0) break;
	}

	if (i == c)
	{
		free(json);
		slog("failed to find projectile '%s'", proj_name);
		return NULL;
	}
	pjson = sj_array_get_nth(ejson, i);

	name = sj_object_get_value_as_string(pjson, "name");
	if (!name)
	{
		free(json);
		slog("missing projectile entity name");
		return NULL;
	}

	array = sj_object_get_value(pjson, "box");
	if (!array)
	{
		free(json);
		slog("missing box object for projectile entity");
		return NULL;
	}
	if (sj_array_get_count(array) != 2)
	{
		free(json);
		slog("missing or extra box parameters for projectile entity");
		return NULL;
	}
	if (!sj_get_float_value(sj_array_get_nth(array, 0), &box_w) ||
		!sj_get_float_value(sj_array_get_nth(array, 1), &box_h))
	{
		free(json);
		slog("one or more box parameters are invalid for projectile entity");
		return NULL;
	}
	box = gfc_rect(0, 0, box_w, box_h);

	array = sj_object_get_value(pjson, "sprite");
	if (!array)
	{
		free(json);
		slog("missing sprite object for projectile entity");
		return NULL;
	}
	if (sj_array_get_count(array) != 4)
	{
		free(json);
		slog("missing or extra sprite parameters for projectile entity");
		return NULL;
	}
	filename = sj_get_string_value(sj_array_get_nth(array, 0));
	if (!filename ||
		!sj_get_integer_value(sj_array_get_nth(array, 1), &frame_w) ||
		!sj_get_integer_value(sj_array_get_nth(array, 2), &frame_h) ||
		!sj_get_integer_value(sj_array_get_nth(array, 3), &frames_per_line))
	{
		free(json);
		slog("one or more sprite parameters are invalid for projectile entity");
		return NULL;
	}
	sprite = gf2d_sprite_load_all(filename, frame_w, frame_h, frames_per_line, 1);

	if (!sj_object_get_value_as_float(pjson, "fall_speed", &fall_speed))
	{
		free(json);
		slog("missing fall_speed object for player entity");
		return NULL;
	}

	free(json);

	self = entity_new();
	if (!self)
	{
		slog("failed to spawn a projectile entity");
		return NULL;
	}
	gfc_line_cpy(self->name, name);
	self->box = box;
	self->sprite = sprite;
	self->fall_speed = fall_speed;
	return self;
}

Entity *projectile_new(Entity *owner, GFC_Vector2D dir, const char *proj_name)
{
	Entity* self;
	ProjectileData* data;

	self = projectile_load(proj_name);
	if (!self) return NULL;

	// projectile defaults
	self->layer = EL_PROJECTILE;
	self->mask = PROJECTILE_MASK;
	
	self->frame = 0;

	self->position = owner->position;
	if (gfc_strlcmp(self->name, "projectile_shuriken") == 0)
	{
		gfc_vector2d_add(self->position, self->position, gfc_vector2d(dir.x * 35, 0));
		gfc_vector2d_scale(self->velocity, dir, 12);
	}
	else if (gfc_strlcmp(self->name, "projectile_bullet") == 0)
	{
		gfc_vector2d_scale(self->velocity, dir, 8);
	}
	self->newPosition = self->position;

	self->range = 1000;
	self->owner = owner;

	self->think = projectile_think;
	self->update = projectile_update;
	self->free = projectile_free;

	// projectile data
	data = gfc_allocate_array(sizeof(ProjectileData), 1);
	if (data)
	{
		self->data = data;
		data->made_contact = 0;
		data->proj_kill = 0;
		data->distance = 0;
	}
	return self;
}

void projectile_think(Entity *self)
{
	ProjectileData* data;

	if ((!self) || (!self->data)) return;
	data = (ProjectileData*) self->data;

	projectile_get_touch_updates(self);

	// get current velocity, then get new position
	physics_get_velocity_based_on_collision(self->box, &self->velocity, &self->fall_speed, PCT_STICK, &data->made_contact);
	gfc_vector2d_add(self->newPosition, self->newPosition, self->velocity);
}

void projectile_update(Entity *self)
{
	int i, c;
	Entity *other;
	ProjectileData *data;
	Window *win;

	if ((!self) || (!self->data)) return;
	data = (ProjectileData*) self->data;
	win = window_find_by_name("objectives_menu");

	// update physics
	self->position = self->newPosition;

	// update objective #2
	if (destroy_rope && destroy_stalagmite) objective_complete(win, 2);

	// destroy projectile if certain conditions are met
	if (data->distance >= self->range || data->proj_kill)
	{
		entity_free(self);
		return;
	}
	if (gfc_strlcmp(self->name, "projectile_bullet") == 0 && data->made_contact)
	{
		entity_free(self);
		return;
	}
}

void projectile_free(Entity *self)
{
	ProjectileData* data;

	if ((!self) || (!self->data)) return;
	data = (ProjectileData*) self->data;
	free(data);
}

void projectile_get_touch_updates(Entity *self)
{
	int i, c;
	Entity *other;
	ProjectileData *data;
	Window *win;

	if ((!self) || (!self->data)) return;
	data = (ProjectileData*) self->data;
	win = window_find_by_name("objectives_menu");

	entity_get_entity_touches(self); // get entities touched this frame

	c = gfc_list_get_count(self->entity_touches);
	for (i = 0; i < c; i++)
	{
		other = gfc_list_get_nth(self->entity_touches, i);
		if (!other) continue;
		if (other->layer == EL_WORLD)
		{
			if (gfc_strlcmp(other->name, "object_grass") == 0) continue;
			else if (gfc_strlcmp(other->name, "object_elevator") == 0) continue;
			else if (gfc_strlcmp(other->name, "object_lamp") == 0) continue;
			else data->proj_kill = 1;

			if (gfc_strlcmp(other->name, "object_rope") == 0) destroy_rope = 1;
			if (gfc_strlcmp(other->name, "object_bad_stalagmite") == 0) destroy_stalagmite = 1;
		}
	}
}