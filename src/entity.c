#include "simple_logger.h"
#include "gf2d_draw.h"
#include "gfc_shape.h"
#include "camera.h"
#include "entity.h"

extern const Bool f_collision_draw;

typedef struct
{
	Entity  *entity_list;
	Uint32	entity_max;
	Uint32  entity_pool;
} EntityManager;

/*
* @brief close the entity system
*/
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
	slog("entity system initialized");
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
	slog("entity system closed");
}

void entity_clear_all(Entity *ignore)
{
	int i;
	for (i = 0; i < _entity_manager.entity_max; i++)
	{
		if (&_entity_manager.entity_list[i] == ignore) continue;
		if (!_entity_manager.entity_list[i]._inuse) continue;
		entity_free(&_entity_manager.entity_list[i]);
	}
}

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
		_entity_manager.entity_list[i].id = ++_entity_manager.entity_pool;
		//set defaults
		_entity_manager.entity_list[i].scale.x = 1;
		_entity_manager.entity_list[i].scale.y = 1;
		return &_entity_manager.entity_list[i];
	}
	slog("no more available entities");
	return NULL;
}

Entity* entity_get_by_id(Uint32 id)
{
	int i;
	if (!_entity_manager.entity_list)
	{
		slog("entity system has not been initialized");
		return NULL;
	}
	for (i = 0; i < _entity_manager.entity_max; i++)
	{
		if (!_entity_manager.entity_list[i]._inuse) continue;
	}
	return NULL;
}

void entity_free(Entity *self)
{
	if (!self) return;
	self->_inuse = 0; // save this spot for future entities
	gf2d_sprite_free(self->sprite);

	// anything else we allocate for our entity would get cleaned up here
	if (self->free) self->free(self);
}

void entity_think(Entity *self)
{
	if (!self) return;
	if (self->think) self->think(self);
}

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

void entity_system_update()
{
	int i;
	for (i = 0; i < _entity_manager.entity_max; i++)
	{
		if (!_entity_manager.entity_list[i]._inuse) continue;
		entity_update(&_entity_manager.entity_list[i]);
	}
}

void entity_draw(Entity *self)
{
	GFC_Vector2D	position, offset;
	GFC_Rect		rect;

	if (!self) return;
	offset = camera_get_offset();
	gfc_vector2d_add(position, self->position, offset);
	if (self->sprite)
	{
		position = gfc_vector2d(position.x - (0.5 * self->sprite->frame_w), position.y - (0.5 * self->sprite->frame_h));
		gf2d_sprite_render(
			self->sprite,
			position,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			(Uint32)self->frame);

		// draw bounding boxes if the collision draw flag is raised
		if (f_collision_draw)
		{
			rect = gfc_rect(position.x, position.y, self->sprite->frame_w, self->sprite->frame_h);
			gf2d_draw_rect(rect, GFC_COLOR_MAGENTA);
		}
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