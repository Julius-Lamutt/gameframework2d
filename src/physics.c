#include <string.h>
#include "simple_logger.h"
#include "simple_json.h"
#include "collision.h"
#include "physics.h"

typedef struct
{
	Uint32		tile_count;
	GFC_Rect	*physics_layer;
	GFC_Circle	*light_physics;
} PhysicsSystem;

PhysicsSystem physics_system = {0};

const float gravity = 0.13;

void physics_init()
{
	atexit(physics_cleanup);
}

void physics_update_world_data(Uint32 tile_count, GFC_Rect *physics_layer, Light *lights)
{
	int i, c;
	GFC_List *light_physics;
	Light *light;

	if (!physics_layer || !lights) return;

	light_physics = gfc_list_new();
	if (!light_physics)
	{
		slog("failed to allocate a list for light physics");
		return;
	}

	c = gfc_list_get_count(lights);
	for (i = 0; i < c; i++)
	{
		GFC_Circle *circle = gfc_allocate_array(sizeof(GFC_Circle), 1);

		light = gfc_list_get_nth(lights, i);
		if (!light) continue;

		circle->x = light->pos.x;
		circle->y = light->pos.y;
		circle->r = light->r1 * 64;
		gfc_list_append(light_physics, circle);
	}

	physics_system.tile_count = tile_count;
	physics_system.physics_layer = physics_layer;
	physics_system.light_physics = light_physics;
}

void physics_cleanup()
{
	if (physics_system.light_physics)
	{
		gfc_list_foreach(physics_system.light_physics, (gfc_work_func*)free);
		gfc_list_delete(physics_system.light_physics);
	}

	memset(&physics_system, 0, sizeof(PhysicsSystem));
	slog("physics system closed");
}

void physics_update_move_state(GFC_Rect box, Uint32 *move_state)
{
	Uint8 temp_state;

	temp_state = collide_with_world_floor_or_ceiling(physics_system.tile_count, physics_system.physics_layer, box);
	if (*move_state == 2) *move_state = temp_state;
	else if (*move_state == 1 && temp_state == 0) *move_state = temp_state;
	else return;
}

void physics_get_velocity(GFC_Rect box, GFC_Vector2D *velocity, float *fall_speed)
{
	physics_get_velocity_based_on_collision(box, &*velocity, &*fall_speed, 0, NULL);
}

void physics_get_velocity_based_on_collision(GFC_Rect box, GFC_Vector2D *velocity, float *fall_speed, Uint32 type, Uint8 *made_contact)
{
	GFC_Vector2D collision;

	// apply gravity if entity can fall
	if (*fall_speed != -1)
	{
		velocity->y += gravity;
		if (velocity->y > *fall_speed) velocity->y = *fall_speed;
	}
	
	// test for collision with world/entity
	collision = collide_with_world(physics_system.tile_count, physics_system.physics_layer, box, *velocity);

	// if made_contact flag, update the flag
	if (made_contact && *made_contact == 0)
	{
		if (collision.x || collision.y) *made_contact = 1;
	}

	if (type == 0) // default
	{
		if (collision.x == 1) velocity->x = 0;
		if (collision.y == 1) velocity->y = 0;
	}
	else if (type == 1)
	{
		if (collision.x && collision.y) velocity->x = velocity->y = *fall_speed = 0;
	}
	else if (type == 2)
	{
		if (collision.x == 1) velocity->x = 0;
		if (collision.y == 1) velocity->y = 0;
	}
	else if (type == 3) return;
	else slog("invalid collision type");
}

Uint8 physics_wall_between_points(GFC_Vector2D a, GFC_Vector2D b)
{
	if (a.x < b.x)
	{
		return collide_with_world_line(physics_system.tile_count, physics_system.physics_layer, a, b);
	}
	else
	{
		collide_with_world_line(physics_system.tile_count, physics_system.physics_layer, b, a);
	}
}

Uint8 physics_object_in_light(GFC_Rect box)
{
	return collide_with_light(physics_system.light_physics, box);
}
