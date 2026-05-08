#include "simple_logger.h"
#include "ai.h"

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

