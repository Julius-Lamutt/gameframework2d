#include "simple_logger.h"
#include "gfc_input.h"
#include "gfc_audio.h"
#include "element_label.h"
#include "element_actor.h"
#include "element_button.h"
#include "elements.h"

/*
* @brief load a window element
* @param element: the window element to load
* @return NULL on error, a pointer to an element otherwise
*/
static Element *element_load(SJson *windel);

/*
* @brief draw an element
* @param element: the element to draw
*/
static void element_draw(Element *element);

/*
* @brief free an element from memory
* @param element: the element to be freed
*/
static void element_free(Element *element);

static Element *element_load(SJson *windel)
{
	const char *name, *type_name;
	int type, can_focus;
	GFC_Vector4D bound_dims;
	GFC_Rect bounds;
	GFC_Color color;
	SJson *array;
	Element *element;

	if (!windel)
	{
		slog("failed to find window element");
		return NULL;
	}
	element = gfc_allocate_array(sizeof(Element), 1);
	if (!element)
	{
		slog("failed to allocate a new element");
		return NULL;
	}

	name = sj_object_get_value_as_string(windel, "name");
	if (!name)
	{
		free(element);
		slog("failed to find name object for window element");
		return NULL;
	}

	type_name = sj_object_get_value_as_string(windel, "type");
	if (!type_name)
	{
		free(element);
		slog("failed to find type object for window element '%s'", name);
		return NULL;
	}
	if (gfc_strlcmp(type_name, "label") == 0) type = ET_LABEL;
	else if (gfc_strlcmp(type_name, "actor") == 0) type = ET_ACTOR;
	else if (gfc_strlcmp(type_name, "button") == 0) type = ET_BUTTON;
	else if (gfc_strlcmp(type_name, "entry") == 0) type = ET_ENTRY;
	else
	{
		free(element);
		slog("invalid type object for window element '%s'", name);
		return NULL;
	}
	
	if (!sj_object_get_value_as_int(windel, "can_focus", &can_focus))
	{
		free(element);
		slog("failed to find can_focus object for window element '%s'", name);
		return NULL;
	}

	array = sj_object_get_value(windel, "bounds");
	if (!array)
	{
		free(element);
		slog("failed to find bounds object for window element '%s'", name);
		return NULL;
	}
	if (sj_array_get_count(array) != 4)
	{
		free(element);
		slog("missing or extra bound dimensions for window element '%s'", name);
		return NULL;
	}
	if (!sj_get_float_value(sj_array_get_nth(array, 0), &bound_dims.x) ||
		!sj_get_float_value(sj_array_get_nth(array, 1), &bound_dims.y) ||
		!sj_get_float_value(sj_array_get_nth(array, 2), &bound_dims.z) ||
		!sj_get_float_value(sj_array_get_nth(array, 3), &bound_dims.w))
	{
		free(element);
		slog("one or more bound dimensions are invalid for window element '%s'", name);
		return NULL;
	}
	bounds = gfc_rect(bound_dims.x, bound_dims.y, bound_dims.z, bound_dims.w);

	array = sj_object_get_value(windel, "color");
	if (!array)
	{
		free(element);
		slog("failed to find color object for window element '%s'", name);
		return NULL;
	}
	if (sj_array_get_count(array) != 4)
	{
		free(element);
		slog("missing or extra color parameters for window element '%s'", name);
		return NULL;
	}
	if (!sj_get_float_value(sj_array_get_nth(array, 0), &color.r) ||
		!sj_get_float_value(sj_array_get_nth(array, 1), &color.g) ||
		!sj_get_float_value(sj_array_get_nth(array, 2), &color.b) ||
		!sj_get_float_value(sj_array_get_nth(array, 3), &color.a))
	{
		free(element);
		slog("one or more color parameters are invalid for window element '%s'", name);
		return NULL;
	}
	color.ct = CT_RGBA8;

	gfc_line_cpy(element->name, name);
	element->type = type;
	element->state = ES_IDLE;
	element->updated = 1;
	element->can_focus = can_focus;
	element->has_focus = 0;
	element->bounds = bounds;
	element->color = color;

	switch (type)
	{
		case ET_LABEL:
			element->data = element_label_load(windel);
			break;
		case ET_ACTOR:
			element->data = element_actor_load(windel);
			break;
		case ET_BUTTON:
			element->data = element_button_load(windel);
			break;
		case ET_ENTRY:
			break;
	}
	return element;
}

GFC_List *element_list_load(SJson *element_list, Window *win)
{
	int i, c;
	GFC_List *elements;
	SJson *windel;
	Element *element;

	if (!element_list)
	{
		slog("failed to find element list");
		return NULL;
	}
	elements = gfc_list_new();
	if (!elements)
	{
		slog("failed to allocate a list of elements");
		return NULL;
	}
	c = sj_array_get_count(element_list);
	for (i = 0; i < c; i++)
	{
		windel = sj_array_get_nth(element_list, i);
		if (!windel) continue;
		element = element_load(windel);
		if (!element)
		{
			slog("element #%i is invalid", i);
			continue;
			return NULL;
		}
		gfc_list_append(elements, element);
	}
	return elements;
}

static void element_draw(Element *element)
{
	int type;

	if (!element) return;
	type = element->type;

	switch (type)
	{
		case ET_LABEL:
			element_label_draw(element->data, element->bounds);
			break;
		case ET_ACTOR:
			element_actor_draw(element->data, element->bounds);
			break;
		case ET_BUTTON:
			element_button_draw(element->data, element);
			break;
		case ET_ENTRY:
			break;
	}
}

void element_list_draw(GFC_List *element_list)
{
	int i, c;
	Element *element;

	if (!element_list) return;
	c = gfc_list_get_count(element_list);
	for (i = 0; i < c; i++)
	{
		element = gfc_list_get_nth(element_list, i);
		if (!element) continue;
		if (element->state == ES_HIDDEN) continue;
		element_draw(element);
	}
}

static void element_free(Element *element)
{
	int type;
	void *data;

	if (!element || !element->data) return;
	type = element->type;

	switch (type)
	{
	case ET_LABEL:
		data = (LabelElement*) element->data;
		element_label_free(data);
		break;
	case ET_ACTOR:
		data = (ActorElement*) element->data;
		element_actor_free(data);
		break;
	case ET_BUTTON:
		data = (ButtonElement*) element->data;
		element_button_free(data);
		break;
	case ET_ENTRY:
		break;
	}
	free(element);
}

void element_list_free(GFC_List *element_list)
{
	int i, c;
	Element *element;

	if (!element_list) return;
	c = gfc_list_get_count(element_list);
	for (i = 0; i < c; i++)
	{
		element = gfc_list_get_nth(element_list, i);
		if (!element) continue;
		element_free(element);
	}
	gfc_list_delete(element_list);
}

void element_update_state(Element *element)
{
	int mx, my;
	void *data;
	GFC_Sound *sound;

	if (!element) return;

	if (element->type == ET_BUTTON)
	{
		SDL_GetMouseState(&mx, &my);
		data = (ButtonElement*) element->data;

		if (gfc_point_in_rect(gfc_vector2d(mx, my), element->bounds))
		{
			if (gfc_input_mouse_left_pressed())
			{
				sound = gfc_sound_load("audio/button_click.wav", 30, 1);
				gfc_sound_play(sound, 0, 30, -1, -1);
				gfc_sound_free(sound);
				element->state = ES_ACTIVE;
			}
			else if (gfc_input_mouse_right_held()) element->state = ES_ACTIVE;
			else if (gfc_input_mouse_right_pressed())
			{
				sound = gfc_sound_load("audio/button_click.wav", 30, 1);
				gfc_sound_play(sound, 0, 30, -1, -1);
				gfc_sound_free(sound);
			}
			else element->state = ES_HIGHLIGHT;
		}
		else element->state = ES_IDLE;
	}
	else if (element->type == ET_ENTRY)
	{
		return;
	}
}

void element_update_label(Element *element, const char* text)
{
	LabelElement* data;

	if (!element) return;
	if (element->type != ET_LABEL)
	{
		slog("cannot call element_update_label on a non-label");
		return;
	}
	data = (LabelElement*)element->data;
	gfc_line_cpy(data->text, text);
}

void element_update_actor(Element *element, Sprite *sprite, Uint8 frame, GFC_Color *color_shift)
{
	ActorElement *data;
	GFC_Color color;

	if (!element) return;
	if (element->type != ET_ACTOR)
	{
		slog("cannot call element_update_actor on a non-actor");
		return;
	}
	data = (ActorElement*)element->data;
	if (sprite) data->image = sprite;
	data->frame = frame;
	if (color_shift)
	{
		color = *color_shift;
		data->color_shift = color;
	}
}