#include "simple_logger.h"
#include "gfc_shape.h"
#include "gfc_vector.h"
#include "bullet.h"
#include "collision.h"
#include "player.h"

/**
 * @brief run the think function for the player
 */
void player_think(Entity* self);

/**
 * @brief run the update function for the player
 */
void player_update(Entity* self);

/**
 * @brief free the player
 */
void player_free(Entity* self);

Bool pressed = false;
Bool released = true;

Entity *player_new()
{
	Entity		*self;

	self = entity_new();
	if (!self)
	{
		slog("Failed to spawn a player entity");
		return NULL;
	}
	self->layer = EL_PLAYER;
	self->sprite = gf2d_sprite_load_all(
		"images/ed210.png",
		128,
		128,
		16,
		0);
	self->frame = 0;
	self->position = gfc_vector2d(500,200);
	self->newPosition = gfc_vector2d(500,200);
	self->proj = NULL;
	self->think = player_think;
	self->update = player_update;
	self->free = player_free;

	return self;
}

void player_think(Entity* self)
{
	if (!self) return;

	Entity* bullet;
	SDL_Event ev;
	GFC_Vector2D dir = {0};
	Sint32 mx = 0, my = 0;

	// move player toward mouse
	SDL_GetMouseState(&mx, &my);
	if (self->position.x < mx) dir.x = 1;
	if (self->position.y < my) dir.y = 1;
	if (self->position.x > mx) dir.x = -1;
	if (self->position.y > my) dir.y = -1;
	gfc_vector2d_normalize(&dir);
	gfc_vector2d_scale(self->velocity, dir, 3);

	// check for collision
	gfc_vector2d_add(self->newPosition, self->newPosition, self->velocity);
	if (collide_with_world(self, self->world))
	{
		self->velocity = gfc_vector2d(0, 0);
		self->newPosition = self->position;
	}

	while (SDL_PollEvent(&ev) != 0) {
		// check event type
		switch (ev.type)
		{
			case SDL_KEYDOWN:
				// key was pressed
				switch (ev.key.keysym.sym)
				{
					case SDLK_t:
						if (released)
						{
							if (!self->proj)
							{
								bullet = bullet_new(self, self->position);
								self->proj = bullet;
							}
							else
							{
								slog("can you see me");
								bullet = self->proj;
								self->position = bullet->position;
								entity_free(bullet);
								self->proj = NULL;
							}
							pressed = true;
							released = false;
							slog("pressed t");
						}
						break;
				}
				break;

			case SDL_KEYUP:
				// key was released
				switch (ev.key.keysym.sym)
				{
					case SDLK_t:
						released = true;
						slog("released t");
						break;
				}
				break;
		}
	}
}

void player_update(Entity* self)
{
	if (!self) return;
	self->frame += 0.1;
	if (self->frame >= 16.0) self->frame = 0;

	gfc_vector2d_add(self->position, self->position, self->velocity);
}

void player_free(Entity* self)
{
	if (!self) return;
}