#include "simple_logger.h"
#include "collision.h"
#include "physics.h"
#include "monster.h"

static Uint8 ignore_gravity = 0; // prevents falling due to gravity

typedef struct
{
	Uint32		type;		// monster type
	Uint16		health;		// monster health
	Uint16		damage;		// monster damage
	Uint16		light_rad;	// monster light radius
	Light		*light;		// monster light
} MonsterData;

/**
* @brief load a monster from a config file
* @param obj_name: name of the monster to load
* @return NULL on error, a monster otherwise
*/
static Entity *monster_load(const char *obj_name);

/**
* @brief run the think function for the monster
* @param self: the mosnter to think
*/
static void monster_think(Entity *self);

/**
* @brief run the update function for the monster
* @param self: the monster to update
*/
static void monster_update(Entity *self);

/**
* @brief free the monster
* @param self: the monster to free
*/
static void monster_free(Entity *self);

/**
* @brief get the monster's touch updates for this frame
* @param self: the monster to get touch updates for
*/
static void monster_get_touch_updates(Entity *self);

static Entity *monster_load(const char *obj_name)
{
	SJson *json, *pjson, *array;
	const char *name, *filename;
	float box_w, box_h, fall_speed;
	GFC_Rect box;
	Sint32 frame_w, frame_h, frames_per_line;
	Sprite *sprite;
	Entity *self;

	json = sj_load("defs/player.json");
	if (!json)
	{
		slog("failed to load player config file");
		return NULL;
	}
	pjson = sj_object_get_value(json, "player_entity");
	if (!pjson)
	{
		free(json);
		slog("missing player entity object");
		return NULL;
	}

	name = sj_object_get_value_as_string(pjson, "name");
	if (!name)
	{
		free(json);
		slog("missing player entity name");
		return NULL;
	}

	array = sj_object_get_value(pjson, "box");
	if (!array)
	{
		free(json);
		slog("missing box object for player entity");
		return NULL;
	}
	if (sj_array_get_count(array) != 2)
	{
		free(json);
		slog("missing or extra box parameters for player entity");
		return NULL;
	}
	if (!sj_get_float_value(sj_array_get_nth(array, 0), &box_w) ||
		!sj_get_float_value(sj_array_get_nth(array, 1), &box_h))
	{
		free(json);
		slog("one or more box parameters are invalid for player entity");
		return NULL;
	}
	box = gfc_rect(0, 0, box_w, box_h);

	array = sj_object_get_value(pjson, "sprite");
	if (!array)
	{
		free(json);
		slog("missing sprite object for player entity");
		return NULL;
	}
	if (sj_array_get_count(array) != 4)
	{
		free(json);
		slog("missing or extra sprite parameters for player entity");
		return NULL;
	}
	filename = sj_get_string_value(sj_array_get_nth(array, 0));
	if (!filename ||
		!sj_get_integer_value(sj_array_get_nth(array, 1), &frame_w) ||
		!sj_get_integer_value(sj_array_get_nth(array, 2), &frame_h) ||
		!sj_get_integer_value(sj_array_get_nth(array, 3), &frames_per_line))
	{
		free(json);
		slog("one or more sprite parameters are invalid for player entity");
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
		slog("failed to spawn a player entity");
		return NULL;
	}
	gfc_line_cpy(self->name, name);
	self->box = box;
	self->sprite = sprite;
	self->fall_speed = fall_speed;
	return self;
}

Entity *monster_new(GFC_Vector2D position, const char *obj_name)
{
	Entity *self;
	MonsterData *data;

	self = monster_load(obj_name);
	if (!self) return NULL;

	// player defaults
	self->layer = EL_MONSTER;
	self->mask = MONSTER_MASK;

	self->frame = 0;

	self->position = position;
	self->newPosition = self->position;
	self->velocity = gfc_vector2d(0, 0);
	self->move_state = EMS_NONE;

	self->proj = NULL;

	self->think = monster_think;
	self->update = monster_update;
	self->free = monster_free;

	// monster data
	data = gfc_allocate_array(sizeof(MonsterData), 1);
	if (data)
	{
		self->data = data;
		data->health = 100;
		data->damage = 30;
		data->light_rad = 40;
	}
	return self;
}

static void monster_think(Entity *self)
{
	MonsterData *data;

	if (!self || !self->data) return;
	data = (MonsterData*) self->data;

	self->velocity.x = 0;

	// TODO: IMPLEMENT BASIC MONSTER MOVEMENT/ACTIONS

	monster_get_touch_updates(self);

	// get current velocity, then get new position
	if (!ignore_gravity) physics_get_velocity(self->box, &self->velocity, &self->fall_speed);
	else
	{
		float fall_speed = -1;
		physics_get_velocity(self->box, &self->velocity, &fall_speed);
	}
	gfc_vector2d_add(self->newPosition, self->newPosition, self->velocity);
}

static void monster_update(Entity *self)
{
	MonsterData *data;

	if (!self || !self->data) return;
	data = (MonsterData*) self->data;
	
	// update physics
	physics_update_move_state(self->box, &self->move_state);
	self->position = self->newPosition;
	ignore_gravity = 0;
}

static void monster_free(Entity *self)
{
	MonsterData *data;

	if (!self || !self->data) return;
	data = (MonsterData*) self->data;
	free(data);
}

static void monster_get_touch_updates(Entity *self)
{
	int i, c;
	Entity *other;
	
	GFC_Vector2D collision;

	if (!self) return;

	entity_get_entity_touches(self); // get entities touched this frame

	c = gfc_list_get_count(self->entity_touches);
	for (i = 0; i < c; i++)
	{
		other = gfc_list_get_nth(self->entity_touches, i);
		if (!other) continue;
		if (other->layer == EL_PROJECTILE)
		{
			if (gfc_strlcmp(other->name, "projectile_shuriken") == 0)
			{
				entity_free(other);
			}
		}
		else if (other->layer == EL_WORLD)
		{
			if (gfc_strlcmp(other->name, "object_grass") == 0) continue;
			else if (gfc_strlcmp(other->name, "object_elevator") == 0) continue;
			else if (gfc_strlcmp(other->name, "object_lamp") == 0) continue;
			else
			{
				collision = collide_with_entity_vector(self->box, self->velocity, other->box, other->velocity);
				if (collision.y == 1)
				{
					ignore_gravity = 1;
					self->velocity.y = 0;
					self->move_state = collide_with_object_floor_or_ceiling(self->box, other->box);
				}
				if (collision.x == 1 && self->move_state != EMS_GROUNDED) self->velocity.x = 0;
				if (gfc_strlcmp(other->name, "object_elevator_floor") == 0 && self->move_state == EMS_GROUNDED)
				{
					self->velocity.y = other->velocity.y;
				}
			}
		}
	}
}

