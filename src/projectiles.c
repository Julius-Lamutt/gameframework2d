#include "simple_logger.h"
#include "physics.h"
#include "collision.h"
#include "windows.h"
#include "objectives_menu.h"
#include "interactables.h"
#include "actor.h"
#include "item_pickup.h"
#include "projectiles.h"

extern const float gravity;
static int destroy_stalagmite = 0;
static int destroy_rope = 0;

typedef struct
{
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

Entity *projectile_load(const char *proj_name)
{
	SJson *json, *ejson, *pjson, *array;
	int i, c;
	const char *name, *filename;
	float box_w, box_h;
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
	sprite = gf2d_sprite_load_all(filename, frame_w, frame_h, frames_per_line, 0);

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
	return self;
}

Entity *projectile_new(Entity* owner, GFC_Vector2D dir, const char *proj_name)
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
	gfc_vector2d_add(self->position, self->position, gfc_vector2d(dir.x * 30, dir.y * 30));
	self->newPosition = self->position;
	self->box = gfc_rect(self->position.x, self->position.y, self->sprite->frame_w, self->sprite->frame_h);
	gfc_vector2d_scale(self->velocity, dir, 12);
	self->acceleration = gfc_vector2d(0, gravity);
	self->collision = gfc_vector2d(0, 0);

	self->range = 1000;
	self->owner = owner;

	self->think = projectile_think;
	self->update = projectile_update;
	self->free = projectile_free;

	// projectile data
	data = gfc_allocate_array(sizeof(ProjectileData), 1);
	if (data) self->data = data;
	return self;
}

void projectile_think(Entity *self)
{
	ProjectileData* data;

	if (!self) return;

	data = self->data; // get projectile data

	if (self->velocity.x != 0) self->velocity.y += gravity; // gravity
	if (self->velocity.y > 4) self->velocity.y = 4; // max falling speed

	// check new position for world collision
	gfc_vector2d_add(self->newPosition, self->newPosition, self->velocity);
	self->collision = collide_with_world(self->world->tileCount, self->world->physicsLayer, self->box, self->velocity);
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

void projectile_update(Entity *self)
{
	int i, c;
	Entity *other;
	ProjectileData *data;
	Window *win;

	if (!self) return;
	win = window_find_by_name("objectives_menu");

	data = self->data; // get projectile data
	// check for collision with other entities
	c = gfc_list_get_count(self->entity_touches);
	for (i = 0; i < c; i++)
	{
		other = gfc_list_get_nth(self->entity_touches, i);
		if (!other) continue;
		if (other->layer == EL_WORLD)
		{
			if (gfc_strlcmp(other->name, "good_stalagmite") == 0)
			{
				interactable_new(other->position, "bad_stalagmite", "images/stalagmite_cracked.png", 32, 64);
				entity_free(other);
				destroy_stalagmite = 1;
			}
			else if (gfc_strlcmp(other->name, "bad_stalagmite") == 0)
			{
				actor_new(other->position, "actor_stalagmite", "images/stalagmite_cracked.png", 32, 64);
				entity_free(other);
			}
			else if (gfc_strlcmp(other->name, "rope") == 0)
			{
				entity_free(other);
				destroy_rope = 1;
			}
		}
	}

	if (destroy_rope && destroy_stalagmite)
	{
		objective_complete(win, 2);
		destroy_rope = 0;
		destroy_stalagmite = 0;
	}

	self->position = self->newPosition;
	if (data->distance >= self->range || self->collision.x == 1 || self->collision.y == 1)
	{
		item_pickup_new(self->position, "pickup_shuriken");
		entity_free(self);
	}
}

void projectile_free(Entity *self)
{
	ProjectileData* data;

	if ((!self) || (!self->data)) return;
	data = (ProjectileData*) self->data;
	free(data);
}