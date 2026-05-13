#ifndef  __ENTITY_H__
#define  __ENTITY_H__

#include <SDL.h>
#include "gfc_shape.h"
#include "gfc_text.h"
#include "gf2d_sprite.h"
#include "world.h"

/* entity collision detection */
#define NO_MASK			(EL_NONE)
#define PLAYER_MASK		(EL_MONSTER|EL_ITEM|EL_PROJECTILE|EL_WORLD)
#define MONSTER_MASK	(EL_PLAYER|EL_MONSTER|EL_PROJECTILE|EL_WORLD)
#define ITEM_MASK		(EL_PLAYER|EL_WORLD)
#define PROJECTILE_MASK (EL_PLAYER|EL_MONSTER|EL_WORLD)
#define WORLD_MASK		(EL_PLAYER|EL_MONSTER|EL_PROJECTILE)
#define ALL_MASK		(EL_ALL)

#define PLAYER_CLIP		(EL_PLAYER)
#define MONSTER_CLIP    (EL_MONSTER)

typedef enum
{
	EL_NONE = 0,
	EL_PLAYER = 1,
	EL_MONSTER = 2,
	EL_ITEM = 4,
	EL_PROJECTILE = 8,
	EL_WORLD = 16,
	EL_ALL = 31
} EntityLayers;

typedef enum
{
	EMS_NONE,
	EMS_GROUNDED,
	EMS_FALLLING
} EntityMoveState;

typedef struct Entity_S
{
	// basic info for all entities
	GFC_TextLine	name;				/* entity name */
	Uint8			_inuse;				/* no touchy */
	Uint32			id;					/* unique entity id */
	Uint32			layer;				/* draw layer */
	Uint32			mask;				/* collision mask, i.e. what entities can be touched */
	GFC_Rect		box;				/* bounding box */

	// rendering
	Sprite			*sprite;			/* entity sprite */
	GFC_Vector2D    scale;				/* sprite scale */
	Uint8			flip;				/* if true, then flip sprite horizontally */
	float           rotation;			/* sprite rotation */
	Uint8			fade;				/* enable/disable fade effect for sprite draws */
	Uint8			glow;				/* enable/disable glow effect for sprite draws */
	float			frame;				/* sprite sheet frame to be used for this render frame */

	// physics
	GFC_Vector2D	position;			/* current position */
	GFC_Vector2D    newPosition;		/* position to be tested for collisions */
	GFC_Vector2D    velocity;			/* current velocity */
	float			fall_speed;			/* max fall speed */
	Uint32			move_state;			/* grounded, falling, hit ceiling, etc. */		
	GFC_List		*entity_touches;	/* list of entities clipped this frame */

	// projectile info
	float           range;				/* how far a projectile can travel before disappearing */
	struct Entity_S	*owner;				/* entity that shot the projectile */
	struct Entity_S	*proj;				/* projectile entity */
	struct Entity_S	*victim;			/* entity that was hit with the projectile */

	// miscellaneous
	Uint8			hidden;				/* 1 if other entities can see this entity, 0 otherwise */

	// functions & custom data
	void (*think)(struct Entity_S *self);
	void (*update)(struct Entity_S *self);
	void (*free)(struct Entity_S *self);
	void *data;
} Entity;

/*
 * @brief initialize the entity management system and queues up cleaning on exit
 * @param max: the maximum number of entities that can exist at the same time
 */
void entity_system_init(Uint32 max);

/*
 * @brief clean up all active entities
 * @param ignore: do not clean up this entity
 */
void entity_system_clear(Entity *ignore);

/*
 * @brief get a blank entity for use
 * @returns NULL on no more room or error, a blank entity otherwise
 */
Entity *entity_new();

/*
* @brief get an entity by its unique id
* @param id: the id to find the entity with
* @return NULL if not found, an entity otherwise
*/
Entity *entity_get_by_id(Uint32 id);

/*
 * @brief clean up an entity and free its spot for future use
 * @param self: the entity to free
 */
void entity_free(Entity *self);

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

/**
* @brief find an entity object from a sjson array by name
* @param array: the sjson array to search in
* @param obj_name: the name of the entity object to search
* @return NULL if not found, a sjson entity object otherwise
*/
SJson *entity_object_get_by_name(SJson *array, const char *obj_name);

/**
* @brief find the entities touched by an entity this frame
* @param self: the entity to find touches for
*/
void entity_get_entity_touches(Entity *self);

#endif
