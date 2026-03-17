#include "simple_logger.h"
#include "items.h"

static SJson *_itemJson = NULL;
static SJson *_itemDefs = NULL;

/*
* @brief TODO
*/
void items_close();

void items_initialize(const char* filename)
{
	if (!filename)
	{
		slog("no filename provided for item initialization");
		return;
	}
	atexit(items_close);

}

void items_close()
{
	if (_itemJson) sj_free(_itemJson);
	_itemJson = NULL;
	_itemDefs = NULL;
}

SJson* items_get_def_by_name(const char* name)
{

}