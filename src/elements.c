#include "simple_logger.h"
#include "elements.h"

Element *element_load(SJson *windel)
{
	const char *name, *type_name;
	int index, type, can_focus;
	GFC_Rect bounds;
	GFC_Color color;
	SJson *array;
	Element *element;

	if (!windel)
	{
		slog("failed to find window element");
		return NULL;
	}

	name = sj_object_get_value_as_string(windel, "name");
	if (!name)
	{
		slog("failed to find name object for window element");
		return NULL;
	}

	if (!sj_object_get_value_as_int(windel, "index", &index))
	{
		slog("failed to find index object for window element '%s'", name);
		return NULL;
	}

	type_name = sj_object_get_value_as_string(windel, "type");
	if (!type_name)
	{
		slog("failed to find type object for window element '%s'", name);
		return NULL;
	}
	if (gfc_strlcmp(type_name, "label")) type = ET_LABEL;
	if (gfc_strlcmp(type_name, "actor")) type = ET_ACTOR;
	if (gfc_strlcmp(type_name, "button")) type = ET_BUTTON;
	if (gfc_strlcmp(type_name, "entry")) type = ET_ENTRY;
	if (gfc_strlcmp(type_name, "list")) type = ET_LIST;
	else
	{
		slog("invalid type object for window element '%s'", name);
		return NULL;
	}

	if (!sj_object_get_value_as_int(windel, "can_focus", &can_focus));
	{
		slog("failed to find can_focus object for window element '%s'", name);
		return NULL;
	}

	array = sj_object_get_value(windel, "bounds");
	if (!array)
	{
		slog("failed to find bounds object for window element '%s'", name);
		return NULL;
	}
	if (sj_array_get_count(array) != 4)
	{
		slog("missing or extra bound dimensions for window element '%s'", windel);
		return NULL;
	}
	if (!sj_get_float_value(sj_array_get_nth(array, 0), &bounds.x) ||
		!sj_get_float_value(sj_array_get_nth(array, 1), &bounds.y) ||
		!sj_get_float_value(sj_array_get_nth(array, 2), &bounds.w) ||
		!sj_get_float_value(sj_array_get_nth(array, 3), &bounds.h))
	{
		slog("one or more bound dimensions are invalid for window element '%s'", windel);
		return NULL;
	}

	array = sj_object_get_value(windel, "color");
	if (!array)
	{
		slog("failed to find color object for window element '%s'", windel);
		return NULL;
	}
	if (sj_array_get_count(array) != 4)
	{
		slog("missing or extra color parameters for window element '%s'", windel);
		return NULL;
	}
	if (!sj_get_float_value(sj_array_get_nth(array, 0), &color.r) ||
		!sj_get_float_value(sj_array_get_nth(array, 1), &color.g) ||
		!sj_get_float_value(sj_array_get_nth(array, 2), &color.b) ||
		!sj_get_float_value(sj_array_get_nth(array, 3), &color.a))
	{
		slog("one or more color are invalid for window element '%s'", windel);
		return NULL;
	}
	color.ct = CT_RGBA8;
	gfc_line_cpy(element->name, name);
	element->index = index;
	element->type = type;
	element->can_focus = can_focus;
	element->bounds = bounds;
	element->color = color;

	return element;
}

GFC_List *element_list_load(SJson *element_list)
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
			//return NULL;
		}
		gfc_list_append(elements, element);
	}
	return elements;
}