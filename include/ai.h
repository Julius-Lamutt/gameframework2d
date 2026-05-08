#ifndef __AI_H__
#define __AI_H__

#include <SDL.h>

typedef enum
{
	AIMS_IDLE,
	AIMS_WALK_L,
	AIMS_WALK_R,
	AIMS_RUN_L,
	AIMS_RUN_R,
	AIMS_JUMP_L,
	AIMS_JUMP_R,
	AIMS_MAX
} AIMoveState;

typedef enum
{
	AIAS_IDLE,
	AIAS_NORMAL,
	AIAS_CAUTION,
	AIAS_ALERT
} AIAlertStatus;

typedef struct
{
	Uint32 alert_status;
	Uint32 move_state;
} MonsterAI;

/*
* @brief update the monster ai
* @param ai: the ai to update
*/
void ai_update(MonsterAI *ai);

/*
* @brief set the ai move state
* @param ai: the ai to set the move state for
* @param move_state: the new move state for the ai
*/
void ai_set_move_state(MonsterAI *ai, Uint32 move_state);

#endif
