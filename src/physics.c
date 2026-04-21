#include "simple_logger.h"
#include "simple_json.h"
#include "collision.h"
#include "physics.h"

typedef struct
{
	Uint32		tile_count;
	GFC_Rect	*physics_layer;
} PhysicsData;

const float gravity = 0.13;

void physics_update_move_state(Uint32 tile_count, GFC_Rect *physics_layer, GFC_Rect box, Uint32 *move_state)
{
	*move_state = collide_with_world_floor_or_ceiling(tile_count,physics_layer, box);
}

void physics_update_velocity(Uint32 tile_count, GFC_Rect *physics_layer, GFC_Rect box, GFC_Vector2D *velocity, float fall_speed)
{
	GFC_Vector2D collision;

	// apply gravity
	velocity->y += gravity;
	if (velocity->y > fall_speed) velocity->y = fall_speed;
	
	// test for collision with world/entity
	collision = collide_with_world(tile_count, physics_layer, box, *velocity);
	if (collision.x == 1) velocity->x = 0;
	if (collision.y == 1) velocity->y = 0;
}
