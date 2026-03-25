#ifndef __OBJECTIVES_MENU_H__
#define __OBJECTIVES_MENU_H__

#include <SDL.h>
#include "windows.h"

/*
* @brief load the objectives menu
* @return NULL on error, a pointer to the main menu window otherwise
*/
Window *objectives_menu();

/*
* @breif set an objective to be complete
* @param win: the window to change objective data for
* @param objective: the objective to set as completed
*/
void objective_complete(Window *win, int objective);

#endif
