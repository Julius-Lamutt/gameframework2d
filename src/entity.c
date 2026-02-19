#include "simple_logger.h"
#include "entity.h"

typedef struct
{
	Entity* entity_list;
	Uint32	entity_max;
} EntityManager;

void entity_system_close();

static EntityManager _entity_manager = {0};

void entity_system_init(Uint32 max)
{
	if (_entity_manager.entity_list)
	{
		slog("entity manager is aleady active");
		return;
	}
	if (!max)
	{
		slog("cannot initialize entity system with zero entities");
		return;
	}
	_entity_manager.entity_list = gfc_allocate_array(sizeof(Entity), max);
	if (!_entity_manager.entity_list)
	{
		slog("failed to allocate global entity list");
		return;
	}
	_entity_manager.entity_max = max;
	atexit(entity_system_close);
	slog("initialized entity system");
}

void entity_system_close()
{
	int i;
	for (i = 0; i < _entity_manager.entity_max; i++)
	{
		entity_free(&_entity_manager.entity_list[i]);
	}
	free(_entity_manager.entity_list);
	memset(&_entity_manager, 0, sizeof(EntityManager));
	slog("closed entity system");
}

void entity_free(Entity* self);

/**
 * @brief clean up all active entities
 * @param ignore do not clean up this entity
 */
void entity_clear_all(Entity *ignore)
{
	int i;
	for (i = 0; i < _entity_manager.entity_max; i++)
	{
		if (&_entity_manager.entity_list[i] == ignore) continue;
		if (!_entity_manager.entity_list[i]._inuse) continue;
		gf2d_sprite_free(_entity_manager.entity_list[i].sprite);
		if (_entity_manager.entity_list[i].free) _entity_manager.entity_list[i].free(_entity_manager.entity_list[i].data);
	}
}

/**
 * @brief get a blank entity for use
 * @returns NULL on no more room or error, a blank entity otherwise
 */
Entity *entity_new()
{
	int i;
	if (!_entity_manager.entity_list)
	{
		slog("entity system has not been initialized!");
		return NULL;
	}
	for (i = 0; i < _entity_manager.entity_max; i++)
	{
		if (_entity_manager.entity_list[i]._inuse) continue;
		memset(&_entity_manager.entity_list[i], 0, sizeof(Entity));
		_entity_manager.entity_list[i]._inuse = 1;
		//set defaults
		_entity_manager.entity_list[i].scale.x = 1;
		_entity_manager.entity_list[i].scale.y = 1;
		return &_entity_manager.entity_list[i];
	}
	slog("no more available entities");
	return NULL;
}

/**
 * @brief clean up an entity and free its spot for future use
 * @param self the entity to free
 */
void entity_free(Entity *self)
{
	if (!self) return;
	gf2d_sprite_free(self); // HACK: free self, not self->sprite?
	// anything else we allocate for our would get cleaned up here
	if (self->free) self->free(self->data);
}

void entity_think(Entity *self)
{
	if (!self) return;
	if (self->think) self->think(self);
}

/**
 * @brief run the think function for all active entities
 */
void entity_system_think()
{
	int i;
	for (i = 0; i < _entity_manager.entity_max; i++)
	{
		if (!_entity_manager.entity_list[i]._inuse) continue;
		entity_think(&_entity_manager.entity_list[i]);
	}
}

void entity_update(Entity *self)
{
	if (!self) return;
	if (self->update) self->update(self);
}

/**
 * @brief run the update function for all active entities
 */
void entity_system_update()
{
	int i;
	for (i = 0; i < _entity_manager.entity_max; i++)
	{
		if (!_entity_manager.entity_list[i]._inuse) continue;
		entity_update(&_entity_manager.entity_list[i]);
	}
}

/**
 * @brief draw all active entities
 */
void entity_draw(Entity *self)
{
	if (!self) return;
	if (self->sprite)
	{
		gf2d_sprite_render(
			self->sprite,
			self->position,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			(Uint32)self->frame);
	}
}

void entity_system_draw()
{
	int i;
	for (i = 0; i < _entity_manager.entity_max; i++)
	{
		if (!_entity_manager.entity_list[i]._inuse) continue;
		entity_draw(&_entity_manager.entity_list[i]);
	}
}