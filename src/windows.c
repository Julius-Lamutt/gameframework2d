#include "simple_logger.h"
#include "windows.h"

typedef struct
{
	Window  *window_list;
	Uint32	entity_max;
	Uint32  entity_pool;
} WindowManager;

static WindowManager _window_manager = {0};

void window_system_init(Uint32 max)
{

}