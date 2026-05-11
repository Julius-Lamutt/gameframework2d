#include <string.h>
#include "simple_logger.h"
#include "ai.h"

static void ai_close();

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

Sint32 ai_get_player_id()
{
	return level_ai.player_id;
}

void ai_update_monster(MonsterAI *ai)
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
	
	if (ai->toggle == 0 && SDL_GetTicks() - ai->last_move > 2500)
	{
		ai->next_action = AINA_ATTACK;
		ai->last_attack = SDL_GetTicks();
		ai->toggle = 1;
	}
	else if (ai->toggle == 1)
	{
		ai->count++;
		if (ai->count != 2)
		{
			ai->next_action = AINA_ATTACK;
			ai->last_attack = SDL_GetTicks();
		}
		else
		{
			ai->next_action = AINA_MOVE;
			ai->last_move = SDL_GetTicks();
			ai_set_move_state(ai, AIMS_WALK_R);
			ai->toggle = 0;
			ai->count = 0;
		}
	}
}

void ai_set_move_state(MonsterAI *ai, Uint32 move_state)
{
	if (!ai) return;
	if (!(move_state < 0) && !(move_state >= AIMS_MAX)) ai->move_state = move_state;
	else ai->move_state = AIMS_IDLE;
}