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
	Uint32			next_action;	// what should the monster ai do next
	Uint32			move_state;		// how should the monster ai move
	Uint32			last_attack;	// time since last attack (monsters attack anim should last 1 second)
	Uint32			last_move;		// time since last movement (monsters move anim should last 0.5 seconds)
	Uint32			last_search;	// time since last search movement
	Uint32			patrol_time;	// monster ai will move based on their patrol time
	Uint8			at_patrol;		// monster ai is at patrol (don't worry about returning to start position)
	GFC_Vector2D	start_pos;		// start position for returning to patrol
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
* @brief update the level ai
*/
void ai_update();

/*
* @brief get the current ai alert state
* @return the ai alert state
*/
Uint32 ai_get_alert_state();

/*
* @brief get the current ai caution timer
* @return the ai caution timer (in milliseconds)
* note: will return 0 if not in caution phase
*/
Uint32 ai_get_caution_timer();

/*
* @brief set the player id for the ai system
* @param id: the id of the player
*/
void ai_set_player_id(Sint32 id);

/*
* @brief add to list of light object ids
* @param id: the id of the light
*/
void ai_add_light_id(Uint32 id);

/*
* @brief Get the player pos from the ai system.
* @return The player's position. If the player's position cannot be found,
* {0, 0} will be returned instead.
*/
GFC_Vector2D ai_get_player_pos();

/*
* @brief update the monster ai
* @param ai: the ai to update
* @param pos: the position of the monster
* @param view_dir: the current view direction of the monster
* @param night_vision: if true, monster can see in the dark (but not in the light)
*/
void ai_update_monster(MonsterAI *ai, GFC_Vector2D pos, GFC_Vector2D view_dir, Uint8 night_vision);

/*
* @brief set the ai move state
* @param ai: the ai to set the move state for
* @param move_state: the new move state for the ai
*/
void ai_set_move_state(MonsterAI *ai, Uint32 move_state);

#endif
