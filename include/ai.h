#ifndef __AI_H__
#define __AI_H__

#include "gfc_vector.h"

typedef enum
{
	AINA_NONE,
	AINA_MOVE,
	AINA_ATTACK
} AINextAction;

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

typedef struct
{
	Uint32		next_action;	// what should the monster ai do next
	Uint32		move_state;		// how should the monster ai move
	Uint32		last_attack;	// time since last attack (monsters attack anim should last 1 second)
	Uint32		last_move;		// time since last movement (monsters move anim should last 0.5 seconds)
	Uint8		toggle;			// 0 = attack, 1 = move right
	Uint8		count;
} MonsterAI;

/*
* @brief Initialize the level ai system.
* Note: This must be initialized before loading a world for monsters to work!!
*/
void ai_init();

/*
* @brief Cleanup any information for the level ai system.
* Note: This must be called every time a player leaves a loaded level!!
*/
void ai_cleanup();

/*
* @brief set the player id for the ai system
* @param id: the id of the player
*/
void ai_set_player_id(Sint32 id);

/*
* @brief get the player id from the ai system
* @return -1 if the ai does not have the player id, the player id otherwise
*/
Sint32 ai_get_player_id();

/*
* @brief update the monster ai
* @param ai: the ai to update
*/
void ai_update_monster(MonsterAI *ai);

/*
* @brief set the ai move state
* @param ai: the ai to set the move state for
* @param move_state: the new move state for the ai
*/
void ai_set_move_state(MonsterAI *ai, Uint32 move_state);

#endif
