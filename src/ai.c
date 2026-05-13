#include <string.h>
#include "simple_logger.h"
#include "entity.h"
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
*/
static Uint8 ai_monster_can_see(MonsterAI *ai, GFC_Vector2D pos, GFC_Vector2D view_dir);

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
	GFC_Vector2D	last_position;	// last known position of the player
	Sint32			player_id;		// the entity id of the current player (used to get player when needed)
	Uint16			sight_distance; // max distance monsters can see from
	float			sight_cone;		// how big monsters' sight cones are (from 0-1 where 0 is big and 1 is small)
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
	if (!json)
	{
		slog("failed to load ai object for ai system");
		return;
	}
	if (!sj_object_get_value_as_int(ajson, "sight_distance", &sight_distance))
	{
		slog("failed to load sight_distance object for ai system");
		return;
	}
	if (!sj_object_get_value_as_float(ajson, "sight_cone", &sight_cone))
	{
		slog("failed to load sight_cone object for ai system");
		return;
	}

	level_ai.sight_distance = sight_distance;
	level_ai.sight_cone = sight_cone;

	// set defaults
	level_ai.alert_status = AIAS_NORMAL;
	level_ai.last_position = gfc_vector2d(0, 0);
	level_ai.player_id = -1;
	level_ai.last_alert = 0;
	level_ai.last_caution = 0;

	atexit(ai_close);
	slog("ai system initialized");
}

static void ai_close()
{
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
}

void ai_update()
{
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

void ai_update_monster(MonsterAI *ai, GFC_Vector2D pos, GFC_Vector2D view_dir)
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
	if (ai_monster_can_see(ai, pos, view_dir))
	{
		if (level_ai.alert_status != AIAS_ALERT) level_ai.alert_status = AIAS_ALERT;

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
	else if (level_ai.alert_status == AIAS_NORMAL)
	{
		ai->next_action = AINA_MOVE;
		ai->move_state = AIMS_IDLE;
	}
}

static Uint8 ai_monster_can_see(MonsterAI *ai, GFC_Vector2D pos, GFC_Vector2D view_dir)
{
	GFC_Vector2D vec;
	Entity *player;
	float dot;

	if (!ai) return 0;
	player = entity_get_by_id(level_ai.player_id);
	if (!player) return 0;

	if (player->hidden && level_ai.alert_status != AIAS_ALERT) return 0; // is player hidden?
	if (!gfc_vector2d_distance_between_less_than(player->position, pos, level_ai.sight_distance)) return 0; // is player too far?
	if (!physics_object_in_light(player->box)) return 0; // is player in the dark?
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
		if (physics_wall_between_points(pos, gfc_vector2d(pos.x + 20, pos.y))) ai_set_move_state(ai, AIMS_JUMP_R);
	}
	else if (pos.x > level_ai.last_position.x) // player went left
	{
		ai_set_move_state(ai, AIMS_RUN_L);
		if (physics_wall_between_points(pos, gfc_vector2d(pos.x - 20, pos.y))) ai_set_move_state(ai, AIMS_JUMP_L);
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

void ai_set_move_state(MonsterAI *ai, Uint32 move_state)
{
	if (!ai) return;
	if (!(move_state < 0) && !(move_state >= AIMS_MAX)) ai->move_state = move_state;
	else ai->move_state = AIMS_IDLE;
}