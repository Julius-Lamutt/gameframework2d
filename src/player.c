#include "simple_logger.h"
#include "simple_json.h"
#include "gfc_input.h"
#include "gfc_shape.h"
#include "gfc_vector.h"
#include "gfc_audio.h"
#include "ai.h"
#include "inventory_menu.h"
#include "objectives_menu.h"
#include "physics.h"
#include "camera.h"
#include "inventory.h"
#include "projectiles.h"
#include "bullet.h"
#include "drone.h"
#include "collision.h"
#include "monster.h"
#include "player.h"

extern void game_next_level();
extern void game_return_to_menu();

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
	Sint16			health;
	Uint8			takedown;
} ClientData;

Uint8 player_focus = 1; // should camera focus on player right now

static Uint8 ignore_gravity = 0; // prevents falling due to gravity

/**
* @brief load a player from a config file
* @return NULL on error, a player otherwise
*/
static Entity *player_load();

/**
 * @brief run the think function for the player
 * @param self: the player to think
 */
static void player_think(Entity *self);

/**
 * @brief run the update function for the player
 * @param self: the player to update
 */
static void player_update(Entity *self);

/**
 * @brief free the player
 * @param self: the player to free
 */
static void player_free(Entity *self);

/**
* @brief handle player input for this frame
* @param self: the player to handle inputs for
*/
static void player_get_input(Entity *self);

/**
* @brief kill the player
* @param self: the player to kill
*/
static void player_die(Entity *self);

/**
* @brief get the player's touch updates for this frame
* @param self: the player to get touch updates for
*/
static void player_get_touch_updates(Entity *self);

static Entity *player_load()
{
	SJson *json, *pjson, *array;
	const char *name, *filename;
	int health;
	float box_w, box_h, fall_speed;
	GFC_Rect box;
	Sint32 frame_w, frame_h, frames_per_line;
	Sprite *sprite;
	Entity *self;
	ClientData *data;

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
	sprite = gf2d_sprite_load_all(filename, frame_w, frame_h, frames_per_line, 1);

	if (!sj_object_get_value_as_float(pjson, "fall_speed", &fall_speed))
	{
		free(json);
		slog("missing fall_speed object for player entity");
		return NULL;
	}

	if (!sj_object_get_value_as_int(pjson, "health", &health))
	{
		free(json);
		slog("missing health object for player entity");
		return NULL;
	}

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
	self->fall_speed = fall_speed;

	data = gfc_allocate_array(sizeof(ClientData), 1);
	if (!data)
	{
		entity_free(self);
		slog("failed to allocate client data for player");
		return NULL;
	}
	self->data = data;
	data->health = health;

	return self;
}

Entity *player_new(GFC_Vector2D position)
{
	Entity *self;
	ClientData *data;

	self = player_load();
	if (!self || !self->data) return NULL;

	// player defaults
	self->layer = EL_PLAYER;
	self->mask = PLAYER_MASK;

	self->frame = 0;

	self->position = position;
	self->newPosition = self->position;
	self->velocity = gfc_vector2d(0, 0);
	self->move_state = EMS_NONE;

	self->proj = NULL;

	self->hidden = 0;

	self->think = player_think;
	self->update = player_update;
	self->free = player_free;

	// client data
	data = (ClientData*) self->data;
	if (data)
	{
		inventory_init(&data->inventory);
		inventory_load_data(&data->inventory, "defs/inventory_data.json");
		data->active_drone = 0;
		data->smoke_invis = 0;
		data->jump_anim = 0;
		data->takedown = 0;
	}

	// tell ai that the player exists
	ai_set_player_id(self->id);

	return self;
}

static void player_think(Entity* self)
{
	ClientData* data;
	Entity* bullet;
	GFC_Vector2D screen;
	Sint32 mx = 0, my = 0;

	if (!self || !self->data) return;
	data = (ClientData*) self->data;

	// offset mouse to account for camera movement
	SDL_GetMouseState(&mx, &my);
	screen = camera_get_position();
	mx += screen.x;
	my += screen.y;

	self->hidden = 0;
	self->velocity.x = 0;
	self->glow = 0;
	data->takedown = 0;

	// check smoke state
	if (data->smoke_invis)
	{
		self->hidden = 1;
		if (SDL_GetTicks() - data->smoke_invis_start > 5000)
		{
			data->smoke_invis = 0;
			self->fade = 0;
		}
	}
	// check jump state
	if (data->jump_anim)
	{
		if (SDL_GetTicks() - data->jump_anim_start > 5000)
		{
			data->jump_anim = 0;
		}
	}

	// handle all player input, including movement
	player_get_input(self);
	self->move_state = 2; // reset move state

	player_get_touch_updates(self);

	// get current velocity, then get new position
	if (!ignore_gravity) physics_get_velocity(self->box, &self->velocity, &self->fall_speed);
	else
	{
		float fall_speed = -1;
		physics_get_velocity(self->box, &self->velocity, &fall_speed);
	}
	gfc_vector2d_add(self->newPosition, self->newPosition, self->velocity);
}

static void player_update(Entity* self)
{
	Entity* other;
	ClientData* data;
	Window *win;

	if (!self || !self->data) return;
	data = (ClientData*) self->data;
	win = window_find_by_name("objectives_menu");

	// update physics
	physics_update_move_state(self->box, &self->move_state);
	self->position = self->newPosition;
	ignore_gravity = 0;

	// update camera position
	if (player_focus) camera_center_on(self->position);

	// update objective 1 completion
	if (item1 && item2 && item3 && item4 && item5)
	{
		objective_complete(win, 1);
		item1 = 0;
		item2 = 0;
		item3 = 0;
		item4 = 0;
		item5 = 0;
	}

	// update hud
	win = window_find_by_name("inventory");
	inventory_menu_update_health(win, data->health);

	if (data->health <= 0) player_die(self); // if no health, kill player
}

static void player_free(Entity *self)
{
	ClientData* data;

	if ((!self) || (!self->data)) return;
	data = (ClientData*) self->data;
	inventory_save_data(&data->inventory, "defs/inventory_data.json");
	inventory_close(&data->inventory);
	free(data);
}

static void player_get_input(Entity *self)
{
	ClientData *data;
	Entity *bullet;
	GFC_Vector2D dir;
	Window *inv;

	if (!self || !self->data) return;
	data = (ClientData*) self->data;
	inv = window_find_by_name("inventory");

	// super jump
	if (gfc_input_command_pressed("jump") && player_focus && self->move_state == EMS_GROUNDED)
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
		inventory_menu_update_drone(inv);
		if (player_focus) player_focus = 0;
		else player_focus = 1;
	}
	// player movement
	if (player_focus)
	{
		if (gfc_input_command_pressed("up") && self->move_state == EMS_GROUNDED) self->velocity.y = -7;
		if (gfc_input_command_down("right") && !gfc_input_command_down("left")) self->velocity.x = 4;
		if (gfc_input_command_down("left") && !gfc_input_command_down("right")) self->velocity.x = -4;
	}
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
	// throw shuriken
	if (gfc_input_command_pressed("shuriken") && player_focus)
	{
		if (inventory_get_item_by_name(&data->inventory, "tool_shuriken"))
		{
			inventory_remove_item(&data->inventory, "tool_shuriken");
			dir = gfc_vector2d(self->velocity.x, 0);
			gfc_vector2d_normalize(&dir);
			if (dir.x == 0) projectile_new(self, gfc_vector2d(1, 0), "projectile_shuriken");
			else projectile_new(self, dir, "projectile_shuriken");
		}
	}
	// use smoke
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
	// takedown
	if (gfc_input_command_pressed("takedown") && player_focus)
	{
		data->takedown = 1;
	}
}

static void player_die(Entity *self)
{
	GFC_Sound *sound;

	if (!self) return;
	sound = gfc_sound_load("audio/player_die.wav", 30, 2);
	gfc_sound_play(sound, 0, 30, -1, -1);
	gfc_sound_free(sound);
	game_return_to_menu();
}

static void player_get_touch_updates(Entity *self)
{
	int i, c;
	Entity *other;
	ClientData *data;
	Window *win;
	GFC_Vector2D collision;

	if (!self || !self->data) return;
	data = (ClientData*) self->data;
	win = window_find_by_name("objectives_menu");

	entity_get_entity_touches(self); // get entities touched this frame

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
				inventory_add_item(&data->inventory, "tool_shuriken");
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
				game_next_level();
			}
		}
		else if (other->layer == EL_PROJECTILE)
		{
			if (gfc_strlcmp(other->name, "projectile_shuriken") == 0)
			{
				inventory_add_item(&data->inventory, "tool_shuriken");
				entity_free(other);
			}
			else if (gfc_strlcmp(other->name, "projectile_bullet") == 0)
			{
				data->health -= 20;
				entity_free(other);
			}
		}
		else if (other->layer == EL_WORLD)
		{
			if (gfc_strlcmp(other->name, "object_grass") == 0) self->hidden = 1;
			else if (gfc_strlcmp(other->name, "object_elevator") == 0) continue;
			else if (gfc_strlcmp(other->name, "object_lamp") == 0) continue;
			else
			{
				collision = collide_with_entity_vector(self->box, self->velocity, other->box, other->velocity);
				if (collision.y == 1)
				{
					ignore_gravity = 1;
					self->velocity.y = 0;
					self->move_state = collide_with_object_floor_or_ceiling(self->box, other->box);
				}
				if (collision.x == 1 && self->move_state != EMS_GROUNDED) self->velocity.x = 0;
				if (gfc_strlcmp(other->name, "object_elevator_floor") == 0 && self->move_state == EMS_GROUNDED)
				{
					self->velocity.y = other->velocity.y;
				}
			}
		}
		else if (other->layer == EL_MONSTER)
		{
			if (ai_get_alert_state() == 1)
			{
				self->glow = 1;
				if (data->takedown) monster_damage(other, 1000);
			}
		}
	}
}