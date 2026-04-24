#include "simple_logger.h"
#include "gf2d_draw.h"
#include "gfc_shape.h"
#include "collision.h"
#include "camera.h"
#include "entity.h"

extern const Bool f_collision_draw;

typedef struct
{
	Entity  *entity_list;
	Uint32	entity_max;
	Uint32  entity_pool;
} EntityManager;

static EntityManager _entity_manager = {0};

/*
* @brief close the entity system
*/
void entity_system_close();

/*
* @brief run the think function for this entity
* @param self: the entity to think
*/
void entity_think(Entity *self);

/*
* @brief run the update function for this entity
* @param self: the entity to update
*/
void entity_update(Entity *self);

/*
* @brief run the draw function for this entity
* @param self: the entity to draw
*/
void entity_draw(Entity *self);

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
	_entity_manager.entity_pool = 0;
	atexit(entity_system_close);
	slog("entity system initialized");
}

void entity_system_close()
{
	entity_system_clear(NULL);
	if (_entity_manager.entity_list) free(_entity_manager.entity_list);
	memset(&_entity_manager, 0, sizeof(EntityManager));
	slog("entity system closed");
}

void entity_system_clear(Entity *ignore)
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

		//initialize entity collision list
		_entity_manager.entity_list[i].entity_touches = gfc_list_new();
		if (!_entity_manager.entity_list[i].entity_touches)
		{
			slog("failed to create a list for array touches");
			return NULL;
		}

		//set defaults
		_entity_manager.entity_list[i]._inuse = 1;
		_entity_manager.entity_list[i].id = ++_entity_manager.entity_pool;
		_entity_manager.entity_list[i].fade = 0;
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
		if (_entity_manager.entity_list[i].id == id) return &_entity_manager.entity_list[i];
	}
	return NULL;
}

void entity_free(Entity *self)
{
	if (!self) return;
	self->_inuse = 0; // save this spot for future entities
	if (self->sprite) gf2d_sprite_free(self->sprite);
	if (self->entity_touches) gfc_list_delete(self->entity_touches);

	// anything else we allocate for our entity would get cleaned up here
	if (self->free) self->free(self);
}

void entity_think(Entity *self)
{
	int i;
	Entity *other;

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
	GFC_Vector2D position, offset;
	GFC_Color color;

	if (!self) return;
	if (self->sprite)
	{
		offset = camera_get_offset();
		gfc_vector2d_add(position, self->position, offset);
		gfc_vector2d_sub(position, position, gfc_vector2d(0.5 * self->sprite->frame_w, 0.5 * self->sprite->frame_h));

		// center collision box on the sprite's center
		self->box = gfc_rect(position.x + ((self->sprite->frame_w - self->box.w) / 2),
			position.y + ((self->sprite->frame_h - self->box.h) / 2), self->box.w, self->box.h);
		
		color = gfc_color8(255, 255, 255, 255);
		if (self->fade) color = gfc_color8(255, 255, 255, 125);

		gf2d_sprite_render(
			self->sprite,
			position,
			NULL,
			NULL,
			NULL,
			NULL,
			&color,
			NULL,
			(Uint32)self->frame);

		// draw bounding boxes if the collision draw flag is raised
		if (f_collision_draw) gf2d_draw_rect(self->box, GFC_COLOR_MAGENTA);
	}
}

void entity_system_draw()
{
	int i, j;
	for (j = 1; j < EL_ALL; j *= 2)
	{
		for (i = 0; i < _entity_manager.entity_max; i++)
		{
			if (!_entity_manager.entity_list[i]._inuse) continue;
			if (_entity_manager.entity_list[i].layer != j) continue;
			entity_draw(&_entity_manager.entity_list[i]);
		}
	}
}

SJson *entity_object_get_by_name(SJson *array, const char *obj_name)
{
	int i, c;
	SJson *object;

	if (!array) return NULL;

	c = sj_array_get_count(array);
	for (i = 0; i < c; i++)
	{
		const char *name;

		object = sj_array_get_nth(array, i);
		if (!object) continue;
		name = sj_object_get_value_as_string(object, "name");
		if (!name)
		{
			slog("missing entity object name");
			continue;
		}
		if (gfc_strlcmp(name, obj_name) == 0) return object;
	}
	return NULL;
}

void entity_get_entity_touches(Entity *self)
{
	int i;
	Entity *other;

	gfc_list_clear(self->entity_touches);
	for (i = 0; i < _entity_manager.entity_max; i++)
	{
		other = &_entity_manager.entity_list[i];
		if (!other || !other->_inuse) continue;
		if (other == self) continue;

		// test for only certain touches based on collision mask
		switch (other->layer)
		{
			case EL_PLAYER:
				if (!(self->mask & EL_PLAYER)) continue;
				break;

			case EL_MONSTER:
				if (!(self->mask & EL_MONSTER)) continue;
				break;

			case EL_ITEM:
				if (!(self->mask & EL_ITEM)) continue;
				break;

			case EL_PROJECTILE:
				if (!(self->mask & EL_PROJECTILE)) continue;
				break;

			case EL_WORLD:
				if (!(self->mask & EL_WORLD)) continue;
				break;
		}

		if (collide_with_entity(self->box, self->velocity, other->box, other->velocity)) gfc_list_append(self->entity_touches, other);
	}
}