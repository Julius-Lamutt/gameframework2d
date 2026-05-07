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
* @param pos: where to spawn the monster
* @param obj_name: name of the monster to spawn
* @return NULL on error, a monster otherwise
**/
Entity *monster_new(GFC_Vector2D position, const char *obj_name);

#endif
