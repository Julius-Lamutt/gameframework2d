#include <string.h>
#include "simple_logger.h"
#include "entity.h"
#include "world_object.h"
#include "physics.h"
#include "ai.h"

/*
* @brief closes the ai system
*/
static void ai_close();

/*
* @brief determine whether a monster can see the player
* @param ai: the ai to check for player sightings
* @param pos: the position of the monster
* @param view_dir: the viewing direction of the monster (left/right)
* @param night_vision: if true, monster can see in the dark (but not in the light)
* @param heat_vision: if true, monster can see hidden players
*/
static Uint8 ai_monster_can_see(MonsterAI *ai, GFC_Vector2D pos, GFC_Vector2D view_dir, Uint8 night_vision, Uint8 heat_vision);

/*
* @brief move towards player's last known position
* @param ai: the monster ai chasing the player
* @param pos: the position of the monster
*/
static void ai_monster_chase(MonsterAI *ai, GFC_Vector2D pos);

/*
* @brief search the area for the player
* @param ai: the monster ai searching for the player
* @param pos: the position of the monster
*/
static void ai_monster_search(MonsterAI *ai, GFC_Vector2D pos);

/*
* @brief investigate the area if anything is suspicous
* @param ai: the ai to investigate
* @param pos: the position of the monster
* @return 0 if nothing suspicous, 1 otherwise
*/
static Uint8 ai_monster_investigate(MonsterAI *ai, GFC_Vector2D pos);

/*
* @brief normal patrolling (move right - pause - move left - pause - repeat)
* @param ai: the ai to patrol
* @param pos: the position of the monster
*/
static void ai_monster_patrol(MonsterAI *ai, GFC_Vector2D pos);

typedef enum
{
	AIAS_IDLE,
	AIAS_NORMAL,
	AIAS_CAUTION,
	AIAS_ALERT
} AIAlertStatus;

typedef struct
{
	Uint32			alert_status;	// current level AI alert status
	Uint32			last_alert;		// last time a monster saw the player
	Uint32			last_caution;	// last time the level was in caution phase
	Uint32			last_light;		// last time monster flicked a light switch
	GFC_Vector2D	last_position;	// last known position of the player
	Sint32			player_id;		// the entity id of the current player (used to get player when needed)
	Uint16			sight_distance; // max distance monsters can see from
	float			sight_cone;		// how big monsters' sight cones are (from 0-1 where 0 is big and 1 is small)
	Uint32			*lights;		// entity ids of lights that the ai will look for changes in
} LevelAI;

static LevelAI level_ai = {0};

void ai_init()
{
	SJson *json, *ajson;
	int sight_distance;
	float sight_cone;

	json = sj_load("defs/ai.json");
	if (!json)
	{
		slog("failed to load ai config file");
		return;
	}
	ajson = sj_object_get_value(json, "ai");
	if (!ajson)
	{
		sj_free(json);
		slog("failed to load ai object for ai system");
		return;
	}
	if (!sj_object_get_value_as_int(ajson, "sight_distance", &sight_distance))
	{
		sj_free(json);
		slog("failed to load sight_distance object for ai system");
		return;
	}
	if (!sj_object_get_value_as_float(ajson, "sight_cone", &sight_cone))
	{
		sj_free(json);
		slog("failed to load sight_cone object for ai system");
		return;
	}
	sj_free(json);
	level_ai.sight_distance = sight_distance;
	level_ai.sight_cone = sight_cone;

	level_ai.lights = gfc_list_new();
	if (!level_ai.lights)
	{
		slog("failed to allocate a light list");
		return;
	}

	// set defaults
	level_ai.alert_status = AIAS_NORMAL;
	level_ai.last_position = gfc_vector2d(0, 0);
	level_ai.player_id = -1;
	level_ai.last_alert = 0;
	level_ai.last_caution = 0;
	level_ai.last_light = 0;

	atexit(ai_close);
	slog("ai system initialized");
}

static void ai_close()
{
	int i, c;
	Uint32 *id;

	c = gfc_list_get_count(level_ai.lights);
	for (i = 0; i < c; i++)
	{
		id = gfc_list_get_nth(level_ai.lights, i);
		if (id) free(id);
	}

	gfc_list_delete(level_ai.lights);
	memset(&level_ai, 0, sizeof(LevelAI));
	slog("ai system closed");
}

void ai_cleanup()
{
	level_ai.alert_status = AIAS_NORMAL;
	level_ai.last_position = gfc_vector2d(0, 0);
	level_ai.player_id = -1;
	level_ai.last_alert = 0;
	level_ai.last_caution = 0;
	level_ai.last_light = 0;
	gfc_list_foreach(level_ai.lights, (gfc_work_func*)free);
	gfc_list_clear(level_ai.lights);
}

void ai_update()
{
	if (level_ai.alert_status == AIAS_ALERT)
	{
		if (level_ai.last_alert && SDL_GetTicks() - level_ai.last_alert > 5000)
		{
			level_ai.alert_status = AIAS_CAUTION;
			level_ai.last_caution = SDL_GetTicks();
		}
	}
	if (level_ai.alert_status == AIAS_CAUTION)
	{
		if (level_ai.last_caution && SDL_GetTicks() - level_ai.last_caution > 10000)
		{
			level_ai.alert_status = AIAS_NORMAL;
			level_ai.last_caution = 0;
		}
	}
}

Uint32 ai_get_alert_state()
{
	return level_ai.alert_status;
}

Uint32 ai_get_caution_timer()
{
	if (level_ai.last_caution && SDL_GetTicks() - level_ai.last_caution < 10000)
	{
		return 10000 - (SDL_GetTicks() - level_ai.last_caution);
	}
	else return 0;
}

void ai_set_player_id(Sint32 id)
{
	level_ai.player_id = id;
}

GFC_Vector2D ai_get_player_pos()
{
	Entity *player;
	
	player = entity_get_by_id(level_ai.player_id);
	if (!player) return gfc_vector2d(0, 0);
	return player->position;
}

void ai_add_light_id(Uint32 id)
{
	Uint32 *new_id;

	new_id = gfc_allocate_array(sizeof(Uint32), 1);
	*new_id = id;

	gfc_list_append(level_ai.lights, new_id);
}

void ai_update_monster(MonsterAI *ai, GFC_Vector2D pos, GFC_Vector2D view_dir, Uint8 night_vision, Uint8 heat_vision)
{
	if (!ai) return;

	// don't do anything during an attack
	if (SDL_GetTicks() - ai->last_attack < 450)
	{
		ai->next_action = AINA_NONE;
		return;
	}
	// if in a moving animation, keep moving
	else if (SDL_GetTicks() - ai->last_move < 250) return;
	
	// me see player, me attack player
	if (ai_monster_can_see(ai, pos, view_dir, night_vision, heat_vision))
	{
		if (level_ai.alert_status != AIAS_ALERT) level_ai.alert_status = AIAS_ALERT;

		ai->at_patrol = 0;
		ai->patrol_time = 0;

		level_ai.last_alert = SDL_GetTicks();
		level_ai.last_caution = 0;

		level_ai.last_position = ai_get_player_pos();
		if (!(level_ai.last_position.x == 0 && level_ai.last_position.y == 0))
		{
			ai->next_action = AINA_ATTACK;
			ai->last_attack = SDL_GetTicks();
		}
	}
	// me no see player, me chase player
	else if (level_ai.alert_status == AIAS_ALERT)
	{
		ai->next_action = AINA_MOVE;
		ai_monster_chase(ai, pos);
	}
	// me lose player, me search for player
	else if (level_ai.alert_status == AIAS_CAUTION)
	{
		ai->next_action = AINA_MOVE;
		ai_monster_search(ai, pos);
	}
	// me no worried, me investigate
	else if (level_ai.alert_status == AIAS_NORMAL)
	{
		ai->next_action = AINA_MOVE;
		if (!ai_monster_investigate(ai, pos))
		{
			// me no see nothing, me patrol
			ai_monster_patrol(ai, pos);
		}
	}
}

static Uint8 ai_monster_can_see(MonsterAI *ai, GFC_Vector2D pos, GFC_Vector2D view_dir, Uint8 night_vision, Uint8 heat_vision)
{
	GFC_Vector2D vec;
	Entity *player;
	float dot;

	if (!ai) return 0;
	player = entity_get_by_id(level_ai.player_id);
	if (!player) return 0;

	if (player->hidden && level_ai.alert_status != AIAS_ALERT) // is player hidden?
	{
		if (!heat_vision) return 0; // can't see hidden players without heat vision
	}

	if (!gfc_vector2d_distance_between_less_than(player->position, pos, level_ai.sight_distance)) return 0; // is player too far?

	if (!physics_object_in_light(player->box)) // is player in the dark?
	{
		if (!night_vision) return 0; // can't see in the dark without night vision
	}
	else if (night_vision) return 0; // can't see in the light with night vision

	if (physics_wall_between_points(pos, player->position)) return 0; // any walls?

	// determine if player is in monster's vision cone
	gfc_vector2d_sub(vec, pos, entity_get_by_id(level_ai.player_id)->position);
	gfc_vector2d_normalize(&vec);
	dot = view_dir.x * vec.x + view_dir.y *vec.y;
	if (dot > level_ai.sight_cone) return 1;
	return 0;
}

static void ai_monster_chase(MonsterAI *ai, GFC_Vector2D pos)
{
	if (!ai) return;

	if (abs(pos.x - level_ai.last_position.x) < 32) // lost them...
	{
		level_ai.alert_status = AIAS_CAUTION;
		level_ai.last_alert = 0;
		level_ai.last_caution = SDL_GetTicks();
	}
	else if (pos.x < level_ai.last_position.x) // player went right
	{
		ai_set_move_state(ai, AIMS_RUN_R);
		if (physics_wall_between_points(pos, gfc_vector2d(pos.x + 32, pos.y))) ai_set_move_state(ai, AIMS_JUMP_R);
	}
	else if (pos.x > level_ai.last_position.x) // player went left
	{
		ai_set_move_state(ai, AIMS_RUN_L);
		if (physics_wall_between_points(pos, gfc_vector2d(pos.x - 32, pos.y))) ai_set_move_state(ai, AIMS_JUMP_L);
	}
}

static void ai_monster_search(MonsterAI *ai, GFC_Vector2D pos)
{
	int random;

	if (!ai) return;
	random = -1;

	// reset last search time if needed
	if (ai->last_search && SDL_GetTicks() - ai->last_search > 2000) ai->last_search = 0;

	// ai should behave more randomly during a search
	if (!ai->last_search)
	{
		random = (rand() % 5);
		ai->last_search = SDL_GetTicks();
	}

	if (random == 0 || random == 1)
	{
		ai_set_move_state(ai, AIMS_WALK_R);
		if (physics_wall_between_points(pos, gfc_vector2d(pos.x + 32, pos.y))) ai_set_move_state(ai, AIMS_JUMP_R);
	}
	else if (random == 2 || random == 3)
	{
		ai_set_move_state(ai, AIMS_WALK_L);
		if (physics_wall_between_points(pos, gfc_vector2d(pos.x - 32, pos.y))) ai_set_move_state(ai, AIMS_JUMP_L);
	}
	else if (random == 4)
	{
		ai_set_move_state(ai, AIMS_IDLE);
	}
}

static Uint8 ai_monster_investigate(MonsterAI *ai, GFC_Vector2D pos)
{
	int i, c;
	Uint32 *id;
	Entity *entity, *closest = NULL;
	float distance;

	if (!ai) return 0;

	c = gfc_list_get_count(level_ai.lights);
	for (i = 0; i < c; i++)
	{
		id = gfc_list_get_nth(level_ai.lights, i);
		if (!id) continue;
		
		entity = entity_get_by_id(*id);
		if (!entity) continue;
		if (!gfc_vector2d_distance_between_less_than(entity->position, pos, level_ai.sight_distance)) continue;
		if (!closest)
		{
			closest = entity;
			distance = gfc_vector2d_magnitude_between(entity->position, pos);
		}
		else if (gfc_vector2d_distance_between_less_than(entity->position, pos, distance))
		{
			closest = entity;
			distance = gfc_vector2d_magnitude_between(entity->position, pos);
		}
	}
	if (!closest || !closest->data) return 0;

	//slog("light on is: %i", world_object_light_on(closest));
	if (world_object_light_on(closest) == 1) return 0;
	else if (world_object_light_on(closest) == 2)
	{
		slog("not a world light object: abort investigation");
		return 0;
	}
	else
	{
		if (physics_wall_between_points(pos, closest->position)) return 0;

		ai->patrol_time = 0;
		ai->at_patrol = 0;

		if (abs(pos.x - closest->position.x) < 32) // at the light
		{
			if (SDL_GetTicks() - level_ai.last_light > 1000)
			{
				world_object_light_trigger(closest);
				level_ai.last_light = SDL_GetTicks();
			}
		}
		else if (pos.x < closest->position.x) // light is towards the right
		{
			ai_set_move_state(ai, AIMS_WALK_R);
			if (physics_wall_between_points(pos, gfc_vector2d(pos.x + 20, pos.y))) ai_set_move_state(ai, AIMS_JUMP_R);
		}
		else if (pos.x > closest->position.x) // light is towards the left
		{
			ai_set_move_state(ai, AIMS_WALK_L);
			if (physics_wall_between_points(pos, gfc_vector2d(pos.x - 20, pos.y))) ai_set_move_state(ai, AIMS_JUMP_L);
		}
	}
	return 1;
}

static void ai_monster_patrol(MonsterAI *ai, GFC_Vector2D pos)
{
	Uint32 interval;

	if (!ai) return;

	if (ai->at_patrol || abs(pos.x - ai->start_pos.x) < 32) // back to normal patrol
	{
		if (!ai->at_patrol) ai->at_patrol = 1;

		if (ai->patrol_time == 0)
		{
			ai_set_move_state(ai, AIMS_WALK_L);
			ai->patrol_time = SDL_GetTicks();
		}

		interval = SDL_GetTicks() - ai->patrol_time;
		if (interval < 3000) ai_set_move_state(ai, AIMS_IDLE);
		else if (interval < 7000) ai_set_move_state(ai, AIMS_WALK_R);
		else if (interval < 10000) ai_set_move_state(ai, AIMS_IDLE);
		else if (interval < 14000) ai_set_move_state(ai, AIMS_WALK_L);
		else ai->patrol_time = SDL_GetTicks();
	}
	else if (!ai->at_patrol && pos.x < ai->start_pos.x) // patrol is to the right
	{
		ai_set_move_state(ai, AIMS_RUN_R);
		if (physics_wall_between_points(pos, gfc_vector2d(pos.x + 32, pos.y))) ai_set_move_state(ai, AIMS_JUMP_R);
	}
	else if (!ai->at_patrol && pos.x > ai->start_pos.x) // patrol is to the left
	{
		ai_set_move_state(ai, AIMS_RUN_L);
		if (physics_wall_between_points(pos, gfc_vector2d(pos.x - 32, pos.y))) ai_set_move_state(ai, AIMS_JUMP_L);
	}
}

void ai_set_move_state(MonsterAI *ai, Uint32 move_state)
{
	if (!ai) return;
	if (!(move_state < 0) && !(move_state >= AIMS_MAX)) ai->move_state = move_state;
	else ai->move_state = AIMS_IDLE;
}