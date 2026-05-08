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
} LevelAI;

static LevelAI level_ai = {0};

void ai_init()
{
	level_ai.alert_status = AIAS_NORMAL;
	level_ai.last_position = gfc_vector2d(0, 0);
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
}

void ai_update(MonsterAI *ai)
{
	if (!ai) return;
	ai_set_move_state(ai, AIMS_JUMP_R);
}

void ai_set_move_state(MonsterAI *ai, Uint32 move_state)
{
	if (!ai) return;
	if (!(move_state < 0) && !(move_state >= AIMS_MAX)) ai->move_state = move_state;
}