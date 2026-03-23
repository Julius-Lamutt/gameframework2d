#include "simple_logger.h"
#include "simple_json.h"
#include "physics.h"

typedef struct
{
	float frame_rate;
	float last_frame;
	float delta;
} PhysicsData;

PhysicsData physics_data = {0};

const float gravity = 0.13;

void physics_system_init(float frame_rate)
{
	physics_data.frame_rate = frame_rate;
	physics_data.last_frame = SDL_GetTicks();
	slog("physics system initialized");
}

void physics_update_delta()
{
	Uint32 current_frame;

	current_frame = SDL_GetTicks();
	physics_data.delta = (current_frame - physics_data.last_frame) * 0.001 * physics_data.frame_rate;
	physics_data.last_frame = current_frame;
	//slog("delta is: %f", physics_manager.delta);
}
