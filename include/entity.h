#ifndef  __ENTITY_H__
#define  __ENTITY_H__

#include <SDL.h>
#include "gfc_text.h"
#include "gf2d_sprite.h"

typedef struct Entity_S
{
	Uint8			_inuse; // no touchy
	GFC_TextLine	name;
	GFC_Vector2D	position;
	GFC_Vector2D    scale;
	float           rotation;
	Sprite			*sprite;
	float			frame;
	void (*think)(struct Entity_S *self);
	void (*update)(struct Entity_S *self);
	void (*free)(struct Entity_S *self);
	void *data;
} Entity;

/**
 * @brief this initializes the entity management system and queues up cleaning on exit
 * @param max the maximum number of entities that can exist at the same time
 */
void entity_system_init(Uint32 max);

/**
 * @brief clean up all active entities
 * @param ignore do not clean up this entity
 */
void entity_clear_all(Entity* ignore);

/**
 * @brief get a blank entity for use
 * @returns NULL on no more room or error, a blank entity otherwise
 */
Entity* entity_new();

/**
 * @brief clean up an entity and free its spot for future use
 * @param self the entity to free
 */
void entity_free(Entity* self);

/**
 * @brief run the think function for all active entities
 */
void entity_system_think();

/**
 * @brief run the update function for all active entities
 */
void entity_system_update();

/**
 * @brief draw all active entities
 */
void entity_system_draw();

#endif
