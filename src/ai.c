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

	level_ai.alert_status = AIAS_NORMAL;
	level_ai.last_position = gfc_vector2d(0, 0);
	level_ai.player_id = -1;
	level_ai.sight_distance = sight_distance;
	level_ai.sight_cone = sight_cone;
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
	
	if (ai_monster_can_see(ai, pos, view_dir))
	{
		ai->next_action = AINA_ATTACK;
		ai->last_attack = SDL_GetTicks();
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

	// is player hidden?
	if (!gfc_vector2d_distance_between_less_than(player->position, pos, level_ai.sight_distance)) return 0; // is player too far?
	if (physics_wall_between_points(pos, player->position)) return 0; // any walls?

	// determine if player is in monster's vision cone
	gfc_vector2d_sub(vec, pos, entity_get_by_id(level_ai.player_id)->position);
	gfc_vector2d_normalize(&vec);
	dot = view_dir.x * vec.x + view_dir.y *vec.y;
	if (dot > level_ai.sight_cone) return 1;
	else return 0;
}

void ai_set_move_state(MonsterAI *ai, Uint32 move_state)
{
	if (!ai) return;
	if (!(move_state < 0) && !(move_state >= AIMS_MAX)) ai->move_state = move_state;
	else ai->move_state = AIMS_IDLE;
}