#ifndef __INVENTORY_MENU_H__
#define __INVENTORY_MENU_H__

#include "windows.h"

/*
* @brief load the inventory menu
* @return NULL on error, a pointer to the main menu window otherwise
*/
Window *inventory_menu();

/*
* @brief update the shuriken inventory count
* @param win: the window to update inventory data for
* @param count: the new invnetory count
*/
void inventory_menu_update_shuriken(Window *win, Uint8 count);

/*
* @brief update the teleport inventory count
* @param win: the window to update inventory data for
* @param count: the new invnetory count
*/
void inventory_menu_update_teleport(Window *win, Uint8 count);

/*
* @brief update the smoke inventory count
* @param win: the window to update inventory data for
* @param count: the new invnetory count
*/
void inventory_menu_update_smoke(Window *win, Uint8 count);

/*
* @brief update the jump inventory count
* @param win: the window to update inventory data for
* @param count: the new invnetory count
*/
void inventory_menu_update_jump(Window *win, Uint8 count);

/*
* @brief toggle the inuse indicator for the drone image
* @param win: the window to pdate inventory data for
*/
void inventory_menu_update_drone(Window *win);

#endif