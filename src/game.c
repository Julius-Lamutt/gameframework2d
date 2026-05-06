#include <SDL.h>
#include "simple_logger.h"
#include "gf2d_graphics.h"
#include "gf2d_sprite.h"
#include "gfc_input.h"
#include "items.h"
#include "font.h"
#include "shadow_map.h"
#include "physics.h"
#include "windows.h"
#include "camera.h"
#include "entity.h"
#include "main_menu.h"
#include "objectives_menu.h"
#include "editor_menu.h"
#include "level.h"
#include "world.h"

/*
* @brief start running the main game loop
*/
void game_start();

/*
* @brief pause the main game loop
*/
void game_pause();

/*
* @brief exit the game executable
*/
void game_exit();

/*
* @brief start the game from the beginning
*/
void game_new();

/*
* @brief load the game using save data
* @param filename: name of the save file
*/
void game_load(const char *filename);

/*
* @brief transition to the next level
*/
void game_next_level();

/*
* @brief return to the game main menu
*/
void game_return_to_menu();

/*
* @brief go to the editor
*/
void game_start_editor();

/*
* @brief leave the editor and return to the main menu
*/
void game_exit_editor();

/*
* @brief update systems that require updates at fixed intervals
*/
static void game_update();

/*
* @brief render the current frame for the game world
*/
static void game_render();

// game flags
const Bool f_collision_draw = false; // for collision debugging

// game variables
static Uint8 done = 0;                      // closes the window
static Uint8 game = 0;                      // starts/pauses the game

static int mx, my;                          // mouse position variable
static float mf;                            // current mouse frame
static Sprite *mouse;                       // sprite for custom mouse
GFC_Color mouse_color;                      // color for custom mouse

static World *world;                        // current world
static Uint8 level = 0;                     // current level (max of 3)

static Window *win, *obj, *edit;            // windows

int main(int argc, char *argv[])
{
    /*program initializtion*/
    init_logger("gf2d.log",0);
    slog("---==== BEGIN ====---");
    gf2d_graphics_initialize(
        "gf2d",
        1200,
        720,
        1200,
        720,
        gfc_vector4d(0,0,0,255),
        0);
    gf2d_graphics_set_frame_delay(16);
    gf2d_sprite_init(1024);
    font_init();
    window_system_init(64);
	entity_system_init(1024);
    camera_set_size(gfc_vector2d(1200, 720));
    gfc_input_init("defs/config.json");
    items_init("defs/items.json");
    SDL_ShowCursor(SDL_DISABLE);
    
    /*demo setup*/
    win = main_menu();
    obj = objectives_menu();
    obj->hidden = 1;
    mouse = gf2d_sprite_load_all("images/pointer.png",32,32,16,0);
    mouse_color = gfc_color8(200, 30, 30, 255);
    slog("press [escape] to quit");

    /*main game loop*/
    while(!done)
    {
        game_update();
        game_render();

        if (game)
        {
            if (gfc_input_key_pressed("m"))
            {
                if (obj->hidden) obj->hidden = 0;
                else obj->hidden = 1;
            }
            if (gfc_input_key_pressed("BACKSPACE"))
            {
                game_return_to_menu();
            }
        }
        if (gfc_input_key_pressed("ESCAPE")) done = 1;
        //slog("Rendering at %f FPS",gf2d_graphics_get_frames_per_second());
    }
    if (world) world_free(world);

    slog("---==== END ====---");
    return 0;
}

void game_start() {game = 1;}
void game_pause() {game = 0;}
void game_exit() {done = 1;}

void game_new()
{
    game_start();
    level++;
    world = world_load("defs/maps/world.json"); //level_1
    world_setup_camera(world);
    physics_update_world_data(world->tileCount, world->physicsLayer);
}

void game_load(const char *filename)
{

}

void game_next_level()
{
    if (level == 3)
    {
        game_return_to_menu();
        return;
    }

    world_free(world);
    world = NULL;
    entity_system_clear(NULL);

    if (level == 1) world = world_load("defs/maps/level_2.json");
    else if (level == 2) world = world_load("defs/maps/level_3.json");
    else
    {
        slog("oops messed up level transitioning somewhere");
        world_free(world);
        world = NULL;
        game_exit();
    }
    level++;
    world_setup_camera(world);
    physics_update_world_data(world->tileCount, world->physicsLayer);
}

void game_return_to_menu()
{
    game_pause();
    level = 0;
    world_free(world);
    world = NULL;
    entity_system_clear(NULL);
    win = main_menu();
}

void game_start_editor()
{
    edit = editor_menu();
}

void game_exit_editor()
{
    mouse_color = gfc_color8(220, 30, 30, 255);
    win = main_menu();
}

static void game_update()
{
    font_cleanup(); // clean the font cache

    gfc_input_update(); // update inputs

    SDL_GetMouseState(&mx, &my); // update mouse state
    mf += 0.1;
    if (mf >= 16.0) mf = 0;

    // update entity information
    if (game) entity_system_think();
    if (game) entity_system_update();

    window_system_update(); // update window and element states
}

static void game_render()
{
    gf2d_graphics_clear_screen(); // clears drawing buffers
    // all drawing should happen betweem clear_screen and next_frame

    // backgrounds drawn first
    if (game) world_draw(world);

    // entities drawn next
    if (game) entity_system_draw();

    // shadow map next
    if (game) shadow_map_draw(world->shadowMap);

    // UI elements last
    window_system_draw();

    gf2d_sprite_draw(
        mouse,
        gfc_vector2d(mx,my),
        NULL,
        NULL,
        NULL,
        NULL,
        &mouse_color,
        (int)mf);

    gf2d_graphics_next_frame(); // render current draw frame and skip to the next frame
}

/*eol@eof*/