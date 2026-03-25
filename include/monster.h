#ifndef __MONSTER_H__
#define __MONSTER_H__

#include "entity.h"

typedef enum
{
	MT_OFFICER,
	MT_SOLDIER,
	MT_SWAT,
	MT_TECHNICIAN,
	MT_NINJA
} MonsterType;

/**
* @brief spawn a monster
* @param type: the type of monster to create
* return NULL on error, a pointer to the player otherwise
**/
Entity* monster_new(Uint32 type);

#endif
