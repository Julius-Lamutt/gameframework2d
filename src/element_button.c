#include "simple_logger.h"
#include "element_button.h"

ButtonElement *element_button_load(SJson *windel)
{
	ButtonElement *button;
	SJson *button_windel, *array;
	int label, actor, type;
	GFC_Color high_color, press_color;

	if (!windel)
	{
		slog("failed to find window element");
		return NULL;
	}
	button = gfc_allocate_array(sizeof(ButtonElement), 1);
	if (!button)
	{
		slog("failed to allocate button element");
		return NULL;
	}

	button_windel = sj_object_get_value(windel, "label");
	if (!button_windel)
	{
		label = 0;
		button->label = NULL;
	}
	else
	{
		label = 1;
		button->label = element_label_load(button_windel);
		if (!button->label)
		{
			element_button_free(button);
			slog("failed to create a label for button element");
			return NULL;
		}
	}

	button_windel = sj_object_get_value(windel, "actor");
	if (!button_windel)
	{
		actor = 0;
		button->actor = NULL;
	}
	else
	{
		actor = 1;
		button->actor = element_actor_load(button_windel);
		if (!button->actor)
		{
			element_button_free(button);
			slog("failed to create an actor for button element");
			return NULL;
		}
	}

	if (label && actor) type =	BT_BOTH;
	else if (label) type = BT_LABEL;
	else if (actor) type = BT_ACTOR;
	else type = BT_HIDDEN;

	array = sj_object_get_value(windel, "high_color");
	if (sj_array_get_count(array) != 4)
	{
		element_button_free(button);
		slog("missing or extra high_color parameters for button element");
		return NULL;
	}
	if (!sj_get_float_value(sj_array_get_nth(array, 0), &high_color.r) ||
		!sj_get_float_value(sj_array_get_nth(array, 1), &high_color.g) ||
		!sj_get_float_value(sj_array_get_nth(array, 2), &high_color.b) ||
		!sj_get_float_value(sj_array_get_nth(array, 3), &high_color.a))
	{
		element_button_free(button);
		slog("one or more high_color parameters are invalid for button element");
		return NULL;
	}
	high_color.ct = CT_RGBA8;

	array = sj_object_get_value(windel, "press_color");
	if (sj_array_get_count(array) != 4)
	{
		element_button_free(button);
		slog("missing or extra press_color parameters for button element");
		return NULL;
	}
	if (!sj_get_float_value(sj_array_get_nth(array, 0), &press_color.r) ||
		!sj_get_float_value(sj_array_get_nth(array, 1), &press_color.g) ||
		!sj_get_float_value(sj_array_get_nth(array, 2), &press_color.b) ||
		!sj_get_float_value(sj_array_get_nth(array, 3), &press_color.a))
	{
		element_button_free(button);
		slog("one or more press_color parameters are invalid for button element");
		return NULL;
	}
	press_color.ct = CT_RGBA8;

	button->type = type;
	button->high_color = high_color;
	button->press_color = press_color;
	return button;
}

void element_button_free(ButtonElement *button)
{
	if (!button) return;
	if (button->label) element_label_free(button->label);
	if (button->actor) element_actor_free(button->actor);
	free(button);
}

void element_button_draw(ButtonElement *button, GFC_Rect bounds, GFC_Color color)
{
	GFC_Rect text_bounds;

	if (!button) return;
	gf2d_draw_rect_filled(bounds, color);

	if (button->actor) element_actor_draw(button->actor, bounds);
	if (button->label)
	{
		text_bounds.x = bounds.x + (0.1 * bounds.w);
		text_bounds.y = bounds.y;
		text_bounds.w = 0.8 * bounds.w;
		text_bounds.h = bounds.h;
		element_label_draw(button->label, text_bounds);
	}
}