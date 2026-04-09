#include "simple_logger.h"
#include "simple_json.h"
#include "gfc_input.h"
#include "gfc_shape.h"
#include "gfc_vector.h"
#include "windows.h"
#include "objectives_menu.h"
#include "physics.h"
#include "camera.h"
#include "inventory.h"
#include "shuriken.h"
#include "bullet.h"
#include "drone.h"
#include "collision.h"
#include "player.h"

extern const float gravity;
static int item1 = 0;
static int item2 = 0;
static int item3 = 0;
static int item4 = 0;
static int item5 = 0;

typedef struct
{
	Inventory		inventory;
	int				active_drone;
	int				smoke_invis;
	int				smoke_invis_start;
	int				jump_anim;
	int				jump_anim_start;
} ClientData;

int	player_focus = 1;

/**
* @brief load a player from a config file
* @return NULL on error, a player otherwise
*/
Entity *player_load();

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

Entity *player_load()
{
	SJson *json, *pjson, *array;
	const char *name, *filename;
	float box_w, box_h;
	GFC_Rect box;
	Sint32 frame_w, frame_h, frames_per_line;
	Sprite *sprite;
	Entity *self;

	json = sj_load("defs/player.json");
	if (!json)
	{
		slog("failed to load player config file");
		return NULL;
	}
	pjson = sj_object_get_value(json, "player_entity");
	if (!pjson)
	{
		free(json);
		slog("missing player entity object");
		return NULL;
	}

	name = sj_object_get_value_as_string(pjson, "name");
	if (!name)
	{
		free(json);
		slog("missing player entity name");
		return NULL;
	}

	array = sj_object_get_value(pjson, "box");
	if (!array)
	{
		free(json);
		slog("missing box object for player entity");
		return NULL;
	}
	if (sj_array_get_count(array) != 2)
	{
		free(json);
		slog("missing or extra box parameters for player entity");
		return NULL;
	}
	if (!sj_get_float_value(sj_array_get_nth(array, 0), &box_w) ||
		!sj_get_float_value(sj_array_get_nth(array, 1), &box_h))
	{
		free(json);
		slog("one or more box parameters are invalid for player entity");
		return NULL;
	}
	box = gfc_rect(0, 0, box_w, box_h);

	array = sj_object_get_value(pjson, "sprite");
	if (!array)
	{
		free(json);
		slog("missing sprite object for player entity");
		return NULL;
	}
	if (sj_array_get_count(array) != 4)
	{
		free(json);
		slog("missing or extra sprite parameters for player entity");
		return NULL;
	}
	filename = sj_get_string_value(sj_array_get_nth(array, 0));
	if (!filename ||
		!sj_get_integer_value(sj_array_get_nth(array, 1), &frame_w) ||
		!sj_get_integer_value(sj_array_get_nth(array, 2), &frame_h) ||
		!sj_get_integer_value(sj_array_get_nth(array, 3), &frames_per_line))
	{
		free(json);
		slog("one or more sprite parameters are invalid for player entity");
		return NULL;
	}
	sprite = gf2d_sprite_load_all(filename, frame_w, frame_h, frames_per_line, 0);

	free(json);

	self = entity_new();
	if (!self)
	{
		slog("failed to spawn a player entity");
		return NULL;
	}
	gfc_line_cpy(self->name, name);
	self->box = box;
	self->sprite = sprite;
	return self;
}

Entity *player_new(GFC_Vector2D position)
{
	Entity *self;
	ClientData *data;

	self = player_load();
	if (!self) return NULL;

	// player defaults
	self->layer = EL_PLAYER;
	self->mask = PLAYER_MASK;

	self->frame = 0;

	self->position = position;
	self->newPosition = self->position;
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
		data->jump_anim = 0;
	}
	return self;
}

void player_think(Entity* self)
{
	ClientData* data;
	Entity* bullet;
	GFC_Vector2D screen, dir, hook_dir;
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

	// check grappler state
	if (data->jump_anim)
	{
		if (SDL_GetTicks() - data->jump_anim_start > 2000)
		{
			data->jump_anim = 0;
		}
	}

	if (gfc_input_command_pressed("jump") && player_focus && self->collision.y == 1)
	{
		if (inventory_get_item_by_name(&data->inventory, "tool_jump") && !data->jump_anim)
		{
			self->velocity.y = -12;
			inventory_remove_item(&data->inventory, "tool_jump");
			data->jump_anim = 1;
			data->jump_anim_start = SDL_GetTicks();
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
}

void player_update(Entity* self)
{
	int i, c;
	Entity* other;
	ClientData* data;
	Window *win;

	if (!self) return;
	data = self->data;
	win = window_find_by_name("objectives_menu");

	// check for collision with other entities
	c = gfc_list_get_count(self->entity_touches);
	for (i = 0; i < c; i++)
	{
		other = gfc_list_get_nth(self->entity_touches, i);
		if (!other) continue;
		if (other->layer == EL_ITEM)
		{
			if (gfc_strlcmp(other->name, "pickup_shuriken") == 0)
			{
				inventory_add_item(&data->inventory, "tool_shuriken");
				entity_free(other);
				item1 = 1;
			}
			if (gfc_strlcmp(other->name, "pickup_teleporter") == 0)
			{
				inventory_add_item(&data->inventory, "tool_teleporter");
				inventory_add_item(&data->inventory, "tool_teleporter");
				entity_free(other);
				item2 = 1;
			}
			if (gfc_strlcmp(other->name, "pickup_drone") == 0)
			{
				inventory_add_item(&data->inventory, "tool_drone");
				entity_free(other);
				item3 = 1;
			}
			if (gfc_strlcmp(other->name, "pickup_smoke") == 0)
			{
				inventory_add_item(&data->inventory, "tool_smoke");
				inventory_add_item(&data->inventory, "tool_smoke");
				entity_free(other);
				item4 = 1;
			}
			if (gfc_strlcmp(other->name, "pickup_jump") == 0)
			{
				inventory_add_item(&data->inventory, "tool_jump");
				inventory_add_item(&data->inventory, "tool_jump");
				entity_free(other);
				item5 = 1;
			}
			if (gfc_strlcmp(other->name, "pickup_diamond") == 0)
			{
				inventory_add_item(&data->inventory, "diamond");
				entity_free(other);
				if (win) objective_complete(win, 3);
			}
		}
		else if (other->layer == EL_WORLD)
		{
			self->collision = collide_with_entity_vector(self, other);
			if (self->collision.x == 1)
			{
				if (gfc_strlcmp(other->name, "grass") == 0) other->fade = 1;
				else self->newPosition.x = self->position.x;
			}
			if (self->collision.y == 1)
			{
				if (gfc_strlcmp(other->name, "grass") == 0) other->fade = 1;
				else
				{
					self->newPosition.y = self->position.y;
					self->velocity.y = 0;
				}
			}
		}
	}
	if (item1 && item2 && item3 && item4 && item5)
	{
		objective_complete(win, 1);
		item1 = 0;
		item2 = 0;
		item3 = 0;
		item4 = 0;
		item5 = 0;
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