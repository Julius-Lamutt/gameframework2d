#include "simple_logger.h"
#include "gfc_input.h"
#include "gfc_shape.h"
#include "gfc_vector.h"
#include "physics.h"
#include "camera.h"
#include "inventory.h"
#include "shuriken.h"
#include "bullet.h"
#include "drone.h"
#include "collision.h"
#include "player.h"

extern const float gravity;

typedef struct
{
	Inventory	inventory;
	int			active_drone;
	int			smoke_invis;
	int			smoke_invis_start;
	int			stab_anim;
	int         stab_anim_start;
	float		stab_anim_frame;
} ClientData;

int	player_focus = 1;

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

	// player defaults
	self->layer = EL_PLAYER;
	self->sprite = gf2d_sprite_load_all("images/player.png", 64, 96, 1, 0);
	self->frame = 0;
	self->position = gfc_vector2d(500, 1000);
	self->newPosition = self->position;
	self->box = gfc_rect(self->position.x, self->position.y, self->sprite->frame_w, self->sprite->frame_h);
	self->velocity = gfc_vector2d(0, 0);
	self->acceleration = gfc_vector2d(0, gravity);
	self->collision = gfc_vector2d(0, 0);
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
	data->active_drone = 0;
	data->smoke_invis = 0;
	data->stab_anim = 0;
}
return self;
}

void player_think(Entity* self)
{
	Sprite *sprite;
	ClientData* data;
	Entity* bullet;
	GFC_Vector2D screen, dir;
	Sint32 mx = 0, my = 0;

	if (!self) return;
	data = self->data;

	// offset mouse to account for camera movement
	screen = camera_get_position();
	SDL_GetMouseState(&mx, &my);
	mx += screen.x;
	my += screen.y;

	// check smoke state
	if (data->smoke_invis)
	{
		if (SDL_GetTicks() - data->smoke_invis_start > 5000)
		{
			data->smoke_invis = 0;
			self->fade = 0;
		}
	}

	// check stab state
	sprite = gf2d_sprite_load_all("images/stab.png", 32, 32, 1, 0);
	if (data->stab_anim)
	{
		data->stab_anim_frame += 0.1;
		gf2d_sprite_render(sprite, gfc_vector2d(self->position.x + 64, self->position.y), NULL, NULL, NULL, NULL, NULL, NULL, 0);
		if (SDL_GetTicks() - data->stab_anim_start > 3000)
		{
			data->stab_anim = 0;
		}
	}

	// drone action
	if (gfc_input_command_pressed("drone") && !data->active_drone)
	{
		if (inventory_get_item_by_name(&data->inventory, "tool_drone"))
		{
			inventory_remove_item(&data->inventory, "tool_drone");
			drone_new(self);
			data->active_drone = 1;
		}
	}
	if (gfc_input_command_pressed("drone_camera") && data->active_drone)
	{
		if (player_focus) player_focus = 0;
		else player_focus = 1;
	}

	// move the player
	self->velocity.x = 0;
	if (player_focus)
	{
		if (gfc_input_command_down("up") && self->collision.y == 1) self->velocity.y = -7;
		if (gfc_input_command_down("right") && !gfc_input_command_down("left")) self->velocity.x = 4;
		if (gfc_input_command_down("left") && !gfc_input_command_down("right")) self->velocity.x = -4;
	}

	self->velocity.y += gravity; // gravity
	if (self->velocity.y > 7) self->velocity.y = 7; // max falling speed

	// teleport player to bullet
	if (gfc_input_command_released("teleport") && player_focus)
	{
		if (!self->proj)
		{
			if (inventory_get_item_by_name(&data->inventory, "tool_teleporter"))
			{
				inventory_remove_item(&data->inventory, "tool_teleporter");
				bullet = bullet_new(self, self->position);
				self->proj = bullet;
			}
		}
		else
		{
			bullet = self->proj;
			self->newPosition = bullet->position;
			self->velocity = gfc_vector2d(0, 0);
			entity_free(bullet);
			self->proj = NULL;
		}
	}

	// check new position for world collision
	gfc_vector2d_add(self->newPosition, self->newPosition, self->velocity);
	self->collision = collide_with_world(self->world->tileCount, self->world->physicsLayer, self->box, self->velocity);
	if (self->collision.x == 1)
	{
		self->newPosition.x = self->position.x;
	}
	if (self->collision.y == 1)
	{
		self->newPosition.y = self->position.y;
		self->velocity.y = 0;
	}

	dir = gfc_vector2d(self->velocity.x, 0);
	gfc_vector2d_normalize(&dir);
	// throw shuriken
	if (gfc_input_command_pressed("shuriken") && player_focus)
	{
		if (inventory_get_item_by_name(&data->inventory, "tool_shuriken"))
		{
			inventory_remove_item(&data->inventory, "tool_shuriken");
			if (dir.x == 0) shuriken_new(self, gfc_vector2d(1, 0));
			else shuriken_new(self, dir);
		}
	}

	if (gfc_input_command_pressed("smoke") && player_focus)
	{
		if (inventory_get_item_by_name(&data->inventory, "tool_smoke") && !data->smoke_invis)
		{
			inventory_remove_item(&data->inventory, "tool_smoke");
			data->smoke_invis = 1;
			data->smoke_invis_start = SDL_GetTicks();
			self->fade = 1;
		}
	}

	if (gfc_input_command_pressed("stab") && player_focus)
	{
		if (inventory_get_item_by_name(&data->inventory, "tool_kitana") && !data->stab_anim)
		{
			data->stab_anim = 1;
			data->stab_anim_start = SDL_GetTicks();
			data->stab_anim_frame = 0;
		}
	}
}

void player_update(Entity* self)
{
	int i, c;
	Entity* other;
	ClientData* data;

	if (!self) return;
	data = self->data;

	// check for collision with other entities
	c = gfc_list_get_count(self->entity_touches);
	for (i = 0; i < c; i++)
	{
		other = gfc_list_get_nth(self->entity_touches, i);
		if (!other) continue;
		if (gfc_strlcmp(other->name, "pickup_shuriken") == 0)
		{
			inventory_add_item(&data->inventory, "tool_shuriken");
			entity_free(other);
		}
		if (gfc_strlcmp(other->name, "pickup_teleporter") == 0)
		{
			inventory_add_item(&data->inventory, "tool_teleporter");
			inventory_add_item(&data->inventory, "tool_teleporter");
			entity_free(other);
		}
		if (gfc_strlcmp(other->name, "pickup_drone") == 0)
		{
			inventory_add_item(&data->inventory, "tool_drone");
			entity_free(other);
		}
		if (gfc_strlcmp(other->name, "pickup_smoke") == 0)
		{
			inventory_add_item(&data->inventory, "tool_smoke");
			inventory_add_item(&data->inventory, "tool_smoke");
			entity_free(other);
		}
		if (gfc_strlcmp(other->name, "pickup_kitana") == 0)
		{
			inventory_add_item(&data->inventory, "tool_kitana");
			entity_free(other);
		}
	}
	self->position = self->newPosition;
	if (player_focus) camera_center_on(self->position);
}

void player_free(Entity *self)
{
	ClientData* data;

	if ((!self) || (!self->data)) return;
	data = (ClientData*) self->data;
	inventory_close(&data->inventory);
	free(data);
}