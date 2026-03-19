#include "simple_logger.h"
#include "elements.h"

GFC_List *element_load(SJson *element_list)
{
	int i, c, index, type;
	const char *name;
	Uint8 can_focus;
	GFC_Rect bounds;
	GFC_Color color;
	GFC_List *elements;
	SJson *element;

	if (!element_list)
	{
		slog("failed to find element list");
		return NULL;
	}
	elements = gfc_list_new();
	if (!elements)
	{
		slog("failed to allocate a list of elements");
	}
	c = sj_array_get_count(element_list);
	for (i = 0; i < c; i++)
	{
		element = sj_array_get_nth(element_list, i);
		if (!element) continue;

		name = sj_object_get_value_as_string(element, "name");
		if (!name)
		{
			slog("missing name element for element #%i", i);
			continue;
		}

		sj_object_get_value_as_int(element, "index", &index);
		if (!index) return NULL;
	}
	return elements;
}