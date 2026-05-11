#include <string.h>
#include "simple_logger.h"
#include "entity.h"
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
} LevelAI;

static LevelAI level_ai = {0};

void ai_init()
{
	level_ai.alert_status = AIAS_NORMAL;
	level_ai.last_position = gfc_vector2d(0, 0);
	level_ai.player_id = -1;
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
	float dot;

	if (!ai) return;



	gfc_vector2d_sub(vec, pos, entity_get_by_id(level_ai.player_id)->position);

	// determine if player is in monster's vision cone
	gfc_vector2d_normalize(&vec);
	dot = view_dir.x * vec.x + view_dir.y *vec.y;
	if (dot > 0.6) return 1;
	else return 0;
}

void ai_set_move_state(MonsterAI *ai, Uint32 move_state)
{
	if (!ai) return;
	if (!(move_state < 0) && !(move_state >= AIMS_MAX)) ai->move_state = move_state;
	else ai->move_state = AIMS_IDLE;
}