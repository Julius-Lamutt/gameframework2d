#include <string.h>
#include "simple_logger.h"
#include "simple_json.h"
#include "collision.h"
#include "physics.h"

typedef struct
{
	Uint32		tile_count;
	GFC_Rect	*physics_layer;
} PhysicsSystem;

PhysicsSystem physics_system = {0};

const float gravity = 0.13;

/*
* @brief cleanup the physics system
*/
void physics_cleanup();

void physics_init()
{
	atexit(physics_cleanup);
}

void physics_update_world_data(Uint32 tile_count, GFC_Rect *physics_layer)
{
	physics_system.tile_count = tile_count;
	physics_system.physics_layer = physics_layer;
}

void physics_cleanup()
{
	memset(&physics_system, 0, sizeof(PhysicsSystem));
	slog("physics system closed");
}

void physics_update_move_state(GFC_Rect box, Uint32 *move_state)
{
	*move_state = collide_with_world_floor_or_ceiling(physics_system.tile_count, physics_system.physics_layer, box);
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
		if (made_contact && *made_contact) velocity->x = velocity->y = *fall_speed = 0;
	}
	else if (type == 2)
	{
		if (collision.x == 1) velocity->x = 0;
		if (collision.y == 1) velocity->y = 0;
	}
	else if (type == 3) return;
	else slog("invalid collision type");
}

GFC_Vector2D physics_collide_with_world(GFC_Rect box, GFC_Vector2D *velocity)
{
	return collide_with_world(physics_system.tile_count, physics_system.physics_layer, box, *velocity);
}
