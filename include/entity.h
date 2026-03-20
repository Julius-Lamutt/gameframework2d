#ifndef  __ENTITY_H__
#define  __ENTITY_H__

#include <SDL.h>
#include "gfc_shape.h"
#include "gfc_text.h"
#include "gf2d_sprite.h"
#include "world.h"

typedef enum
{
	EL_NONE = 0,
	EL_PLAYER = 1,
	EL_MONSTER = 2,
	EL_ITEM = 4,
	EL_PROJECTILES = 8,
	EL_ALL = 15
} Entity_Layers;

typedef struct Entity_S
{
	Uint8			_inuse;			/* no touchy */
	Uint16			layer;
	Uint32			id;
	GFC_TextLine	name;
	GFC_Vector2D	position;		/* current position */
	GFC_Vector2D    newPosition;	/* position to be tested for collisions */
	GFC_Vector2D    scale;
	GFC_Vector2D    velocity;
	World			*world;			/* current world the player is in */
	Sprite			*sprite;
	float           rotation;
	float			frame;
	struct Entity	*owner;			/* entity that shot the projectile */
	struct Entity	*proj;
	struct Entity	*victim;		/* entity that was hit with the projectile */
	float           range;			/* how far a projectile can travel before disappearing */

	void (*think)(struct Entity_S *self);
	void (*update)(struct Entity_S *self);
	void (*free)(struct Entity_S *self);
	void *data;
} Entity;

/*
 * @brief this initializes the entity management system and queues up cleaning on exit
 * @param max: the maximum number of entities that can exist at the same time
 */
void entity_system_init(Uint32 max);

/*
 * @brief clean up all active entities
 * @param ignore: do not clean up this entity
 */
void entity_clear_all(Entity* ignore);

/*
 * @brief get a blank entity for use
 * @returns NULL on no more room or error, a blank entity otherwise
 */
Entity* entity_new();

/*
 * @brief clean up an entity and free its spot for future use
 * @param self: the entity to free
 */
void entity_free(Entity* self);

/*
 * @brief run the think function for all active entities
 */
void entity_system_think();

/*
 * @brief run the update function for all active entities
 */
void entity_system_update();

/*
 * @brief draw all active entities
 */
void entity_system_draw();

#endif
