#include "simple_logger.h"
#include "gfc_input.h"
#include "gfc_shape.h"
#include "gfc_vector.h"
#include "camera.h"
#include "inventory.h"
#include "bullet.h"
#include "collision.h"
#include "player.h"

typedef struct
{
	Inventory inventory;
} ClientData;

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

Entity *player_new()
{
	Entity *self;
	ClientData *data;

	self = entity_new();
	if (!self)
	{
		slog("Failed to spawn a player entity");
		return NULL;
	}

	// player information
	self->layer = EL_PLAYER;
	self->sprite = gf2d_sprite_load_all(
		"images/player.png", //images/ed210.png
		64, //128
		96, //128
		1, //16
		0);
	self->frame = 0;
	self->position = gfc_vector2d(500,200);
	self->newPosition = gfc_vector2d(500,200);
	self->proj = NULL;
	self->think = player_think;
	self->update = player_update;
	self->free = player_free;

	// client data
	data = gfc_allocate_array(sizeof(ClientData), 1);
	if (data)
	{
		self->data = data;
		inventory_init(&data->inventory);
	}
	return self;
}

void player_think(Entity* self)
{
	Entity* bullet;
	GFC_Vector2D screen;
	GFC_Vector2D dir = {0};
	Sint32 mx = 0, my = 0;

	if (!self) return;

	// offset mouse to account for camera movement
	screen = camera_get_position();
	SDL_GetMouseState(&mx, &my);
	mx += screen.x;
	my += screen.y;

	// teleport player to bullet
	if (gfc_input_command_released("teleport"))
	{
		if (!self->proj)
		{
			bullet = bullet_new(self, self->position);
			self->proj = bullet;
		}
		else
		{
			bullet = self->proj;
			self->position = bullet->position;
			entity_free(bullet);
			self->proj = NULL;
		}
	}

	// move the player
	if (gfc_input_command_down("up")) dir.y -= 1;
	if (gfc_input_command_down("down")) dir.y += 1;
	if (gfc_input_command_down("left")) dir.x -= 1;
	if (gfc_input_command_down("right")) dir.x += 1;
	gfc_vector2d_normalize(&dir);
	gfc_vector2d_scale(self->velocity, dir, 5);

	// check for collision
	gfc_vector2d_add(self->newPosition, self->newPosition, self->velocity);
	if (collide_with_world(self, self->world))
	{
		self->velocity = gfc_vector2d(0, 0);
		self->newPosition = self->position;
	}
}

void player_update(Entity* self)
{
	if (!self) return;
	//self->frame += 0.1;
	//if (self->frame >= 16.0) self->frame = 0;
	gfc_vector2d_add(self->position, self->position, self->velocity);
	camera_center_on(self->position);
}

void player_free(Entity *self)
{
	ClientData* data;

	if ((!self) || (!self->data)) return;
	data = (ClientData*) self->data;
	inventory_close(&data->inventory);
	free(data);
}