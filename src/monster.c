#include "simple_logger.h"
#include "gfc_actions.h"
#include "gfc_audio.h"
#include "collision.h"
#include "physics.h"
#include "projectiles.h"
#include "ai.h"
#include "monster.h"

static Uint8 ignore_gravity = 0; // prevents falling due to gravity

typedef struct
{
	MonsterAI			*ai;			// monster ai
	GFC_ActionList		*actions;		// monster animations
	Uint32				type;			// monster type
	Sint16				health;			// monster health
	Uint16				damage;			// monster damage
	Uint16				light_rad;		// monster light radius
	Uint8				walk_speed;		// walk speed
	Uint8				run_speed;		// run speed
	Uint8				jump_speed;		// jump speed
	Uint8				view_dir;		// 0 = left, 1, = right
	Light				*light;			// monster light
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
* @brief move the monster
* @param self: the monster to move
*/
static void monster_move(Entity *self);

/**
* @brief make the monster shoot
* @param self: the monster to make shoot
*/
static void monster_shoot(Entity *self);

/*
* @brief kill the monster
* @param self: the monster to kill
*/
static void monster_die(Entity *self);

/**
* @brief get the monster's touch updates for this frame
* @param self: the monster to get touch updates for
*/
static void monster_get_touch_updates(Entity *self);

static Entity *monster_load(const char *obj_name)
{
	SJson *json, *ejson, *mjson, *array;
	const char *name, *filename, *type;
	int health, damage, light_rad, monster_type, walk_speed, run_speed, jump_speed;
	float box_w, box_h, fall_speed;
	GFC_Rect box;
	Sint32 frame_w, frame_h, frames_per_line;
	Sprite *sprite;
	Entity *self;
	MonsterData *data;

	json = sj_load("defs/monsters.json");
	if (!json)
	{
		slog("failed to load monster config file");
		return NULL;
	}
	ejson = sj_object_get_value(json, "monster_entity");
	if (!ejson)
	{
		free(json);
		slog("missing monster entity object");
		return NULL;
	}
	mjson = entity_object_get_by_name(ejson, obj_name);
	if (!mjson)
	{
		free(json);
		slog("failed to find monster object '%s'", obj_name);
		return NULL;
	}

	name = sj_object_get_value_as_string(mjson, "name");
	if (!name)
	{
		free(json);
		slog("missing monster entity name");
		return NULL;
	}

	array = sj_object_get_value(mjson, "box");
	if (!array)
	{
		free(json);
		slog("missing box object for monster entity");
		return NULL;
	}
	if (sj_array_get_count(array) != 2)
	{
		free(json);
		slog("missing or extra box parameters for monster entity");
		return NULL;
	}
	if (!sj_get_float_value(sj_array_get_nth(array, 0), &box_w) ||
		!sj_get_float_value(sj_array_get_nth(array, 1), &box_h))
	{
		free(json);
		slog("one or more box parameters are invalid for monster entity");
		return NULL;
	}
	box = gfc_rect(0, 0, box_w, box_h);

	array = sj_object_get_value(mjson, "sprite");
	if (!array)
	{
		free(json);
		slog("missing sprite object for monster entity");
		return NULL;
	}
	if (sj_array_get_count(array) != 4)
	{
		free(json);
		slog("missing or extra sprite parameters for monster entity");
		return NULL;
	}
	filename = sj_get_string_value(sj_array_get_nth(array, 0));
	if (!filename ||
		!sj_get_integer_value(sj_array_get_nth(array, 1), &frame_w) ||
		!sj_get_integer_value(sj_array_get_nth(array, 2), &frame_h) ||
		!sj_get_integer_value(sj_array_get_nth(array, 3), &frames_per_line))
	{
		free(json);
		slog("one or more sprite parameters are invalid for monster entity");
		return NULL;
	}
	sprite = gf2d_sprite_load_all(filename, frame_w, frame_h, frames_per_line, 1);

	if (!sj_object_get_value_as_float(mjson, "fall_speed", &fall_speed))
	{
		free(json);
		slog("missing fall_speed object for mosnter entity");
		return NULL;
	}

	type = sj_object_get_value_as_string(mjson, "type");
	if (!type)
	{
		free(json);
		slog("missing type object for monster entity");
		return NULL;
	}
	if (gfc_strlcmp(type, "officer") == 0) monster_type = MT_OFFICER;
	else if (gfc_strlcmp(type, "soldier") == 0) monster_type = MT_SOLDIER;
	else if (gfc_strlcmp(type, "swat") == 0) monster_type = MT_SWAT;
	else if (gfc_strlcmp(type, "technician") == 0) monster_type = MT_TECHNICIAN;
	else if (gfc_strlcmp(type, "ninja") == 0) monster_type = MT_NINJA;
	else
	{
		free(json);
		slog("invalid type object for monster entity: '%s'", type);
		return NULL;
	}

	if (!sj_object_get_value_as_int(mjson, "health", &health))
	{
		free(json);
		slog("missing health object for monster entity");
		return NULL;
	}

	if (!sj_object_get_value_as_int(mjson, "damage", &damage))
	{
		free(json);
		slog("missing damage object for mosnter entity");
		return NULL;
	}

	if (!sj_object_get_value_as_int(mjson, "light_rad", &light_rad))
	{
		free(json);
		slog("missing light_rad object for monster entity");
		return NULL;
	}

	if (!sj_object_get_value_as_int(mjson, "walk_speed", &walk_speed))
	{
		free(json);
		slog("missing walk_speed object for monster entity");
		return NULL;
	}

	if (!sj_object_get_value_as_int(mjson, "run_speed", &run_speed))
	{
		free(json);
		slog("missing run_speed object for monster entity");
		return NULL;
	}

	if (!sj_object_get_value_as_int(mjson, "jump_speed", &jump_speed))
	{
		free(json);
		slog("missing jump_speed object for monster entity");
		return NULL;
	}

	free(json);

	// set monster basic info
	self = entity_new();
	if (!self)
	{
		slog("failed to spawn a monster entity");
		return NULL;
	}
	gfc_line_cpy(self->name, name);
	self->box = box;
	self->sprite = sprite;
	self->fall_speed = fall_speed;

	// set monster data
	data = gfc_allocate_array(sizeof(MonsterData), 1);
	if (!data)
	{
		entity_free(self);
		slog("failed to allocate data for monster entity");
		return NULL;
	}
	self->data = data;
	data->type = monster_type;
	data->health = health;
	data->damage = damage;
	data->light_rad = light_rad;
	data->walk_speed = walk_speed;
	data->run_speed = run_speed;
	data->jump_speed = jump_speed;
	return self;
}

Entity *monster_new(GFC_Vector2D position, const char *obj_name)
{
	Entity *self;
	MonsterData *data;
	MonsterAI *ai;

	// load config files
	self = monster_load(obj_name);
	if (!self || !self->data) return NULL;

	data = (MonsterData*) self->data;
	data->actions = gfc_action_list_load("defs/actions/monster_actions.json");
	if (!data->actions)
	{
		slog("failed to get monster animations");
		entity_free(self);
		return NULL;
	}

	// monster will not work without AI
	ai = gfc_allocate_array(sizeof(MonsterAI), 1);
	if (!ai)
	{
		slog("failed to allocate AI for monster");
		entity_free(self);
		return NULL;
	}

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
	data->ai = ai;
	ai->move_state = AIMS_IDLE;
	ai->last_attack = 0;
	ai->last_search = 0;
	ai->patrol_time = 0;
	ai->at_patrol = 1;
	ai->start_pos = self->position;

	return self;
}

static void monster_think(Entity *self)
{
	MonsterData *data;
	MonsterAI *ai;

	if (!self || !self->data) return;
	data = (MonsterData*) self->data;

	if (!data->ai) return;
	ai = (MonsterAI*) data->ai;

	self->velocity.x = 0;

	// decide what to do next
	if (ai->next_action == AINA_ATTACK) monster_shoot(self);
	else if (ai->next_action == AINA_MOVE) monster_move(self);

	self->move_state = 2; // reset move state

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
	MonsterAI *ai;
	GFC_Action *action;
	GFC_Vector2D dir;
	Uint8 f_anim;

	if (!self || !self->data) return;
	data = (MonsterData*) self->data;

	if (!data->ai) return;
	ai = (MonsterAI*) data->ai;

	// kill monster if it is out of health
	if (data->health <= 0)
	{
		monster_die(self);
		return;
	}
	
	// update physics
	physics_update_move_state(self->box, &self->move_state);
	self->position = self->newPosition;
	ignore_gravity = 0;

	// check if last shooting animation is finished
	if (SDL_GetTicks() - 450 > ai->last_attack) f_anim = 1;
	else f_anim = 0;

	// update monster ai
	if (self->flip) dir = gfc_vector2d(1, 0);
	else dir = gfc_vector2d(-1, 0);
	ai_update_monster(ai, self->position, dir);

	// update shoting animation if needed
	if (SDL_GetTicks() - 450 < ai->last_attack)
	{
		action = gfc_action_list_get_action(data->actions, "shoot");
		gfc_action_next_frame(action, &self->frame);
	}
	if (f_anim && ai->next_action == AINA_ATTACK) self->frame = 0;
}

static void monster_free(Entity *self)
{
	MonsterData *data;
	MonsterAI *ai;

	if (!self || !self->data) return;
	data = (MonsterData*) self->data;

	if (data->actions) gfc_action_list_free(data->actions);

	if (data->ai)
	{
		ai = (MonsterAI*) data->ai;
		free(ai);
	}

	free(data);
}

static void monster_move(Entity *self)
{
	MonsterData *data;
	MonsterAI *ai;
	GFC_Action *action;

	if (!self || !self->data) return;
	data = (MonsterData*) self->data;

	if (!data->ai) return;
	ai = (MonsterAI*) data->ai;

	switch (ai->move_state)
	{
		case AIMS_IDLE:
			action = gfc_action_list_get_action(data->actions, "idle");
			gfc_action_next_frame(action, &self->frame);
			return;

		case AIMS_WALK_L:
			self->flip = 1;
			action = gfc_action_list_get_action(data->actions, "walk");
			gfc_action_next_frame(action, &self->frame);
			self->velocity.x = -data->walk_speed;
			break;

		case AIMS_WALK_R:
			self->flip = 0;
			action = gfc_action_list_get_action(data->actions, "walk");
			gfc_action_next_frame(action, &self->frame);
			self->velocity.x = data->walk_speed;
			break;

		case AIMS_RUN_L:
			self->flip = 1;
			action = gfc_action_list_get_action(data->actions, "run");
			gfc_action_next_frame(action, &self->frame);
			self->velocity.x = -data->run_speed;
			break;

		case AIMS_RUN_R:
			self->flip = 0;
			action = gfc_action_list_get_action(data->actions, "run");
			gfc_action_next_frame(action, &self->frame);
			self->velocity.x = data->run_speed;
			break;

		case AIMS_JUMP_L:
			self->flip = 1;
			action = gfc_action_list_get_action(data->actions, "jump");
			gfc_action_next_frame(action, &self->frame);
			self->velocity.x = -2;
			if (self->move_state == EMS_GROUNDED) self->velocity.y = -data->jump_speed;
			break;

		case AIMS_JUMP_R:
			self->flip = 0;
			action = gfc_action_list_get_action(data->actions, "jump");
			gfc_action_next_frame(action, &self->frame);
			self->velocity.x = 2;
			if (self->move_state == EMS_GROUNDED) self->velocity.y = -data->jump_speed;
			break;
	}
}

static void monster_shoot(Entity *self)
{
	MonsterData *data;
	MonsterAI *ai;
	GFC_Vector2D dir;
	GFC_Sound *sound;

	if (!self || !self->data) return;
	data = (MonsterData*) self->data;

	if (!data->ai) return;
	ai = (MonsterAI*) data->ai;

	gfc_vector2d_sub(dir, ai_get_player_pos(), self->position);
	gfc_vector2d_normalize(&dir);
	projectile_new(self, dir, "projectile_bullet");
	sound = gfc_sound_load("audio/gunshot.wav", 30, 3);
	gfc_sound_play(sound, 0, 30, -1, -1);
	gfc_sound_free(sound);
}

static void monster_die(Entity *self)
{
	GFC_Sound *sound;

	if (!self) return;
	sound = gfc_sound_load("audio/monster_die.wav", 30, 2);
	gfc_sound_play(sound, 0, 30, -1, -1);
	gfc_sound_free(sound);
	entity_free(self);
}

static void monster_get_touch_updates(Entity *self)
{
	int i, c;
	Entity *other;
	GFC_Vector2D collision;
	MonsterData *data;
	MonsterAI *ai;
	GFC_Action *action;

	if (!self || !self->data) return;
	data = (MonsterData*) self->data;

	if (!data->ai) return;
	ai = (MonsterAI*) data->ai;

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
				data->health -= 100;
				entity_free(other);
			}
		}
		else if (other->layer == EL_WORLD)
		{
			if (gfc_strlcmp(other->name, "object_grass") == 0) continue;
			else if (gfc_strlcmp(other->name, "object_elevator") == 0) continue;
			else if (gfc_strlcmp(other->name, "object_lamp") == 0) continue;
			else if (gfc_strlcmp(other->name, "object_bad_stalagmite_fall") == 0)
			{
				monster_die(self);
				return;
			}
			else if (gfc_strlcmp(other->name, "object_light_fall") == 0)
			{
				monster_die(self);
				return;
			}
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

