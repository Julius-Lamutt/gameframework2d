#include <SDL.h>
#include "simple_logger.h"
#include "gfc_input.h"
#include "gf2d_graphics.h"
#include "gf2d_sprite.h"
#include "physics.h"
#include "font.h"
#include "windows.h"
#include "camera.h"
#include "entity.h"
#include "interactables.h"
#include "items.h"
#include "item_pickup.h"
#include "player.h"
#include "bullet.h"
#include "main_menu.h"
#include "objectives_menu.h"
#include "world.h"

/*
* @brief start the demo
*/
void start_game();

/*
* @brief exit the demo
*/
void exit_game();

/*game flags*/
const Bool f_collision_draw = false; // for collision debugging

/*game variables*/
static int done = 0;
static int game = 0;

int main(int argc, char *argv[])
{
    /*variable declarations*/
    int mx,my;
    float mf = 0;
    World *world;
    Window *win, *obj;
    Entity *player, *item1, *item2, *item3, *item4, *item5, *item6, *stalagmite, *rope, *grass;
    Sprite *mouse;
    GFC_Color mouseGFC_Color = gfc_color8(225, 30, 30, 200);
    
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
    physics_system_init(60.0);
	entity_system_init(1024);
    camera_set_size(gfc_vector2d(1200, 720));
    gfc_input_init("defs/config.json");
    items_init("defs/items.json");

    SDL_ShowCursor(SDL_DISABLE);
    
    /*demo setup*/
    world = world_load("defs/maps/testworld.json");
    world_setup_camera(world);
    //player = player_new();
    item1 = item_pickup_new(gfc_vector2d(600, 1200), "pickup_shuriken", "images/shuriken.png", 16, 16);
    item2 = item_pickup_new(gfc_vector2d(700, 1200), "pickup_teleporter", "images/toolbox_blue.png", 32, 32);
    item3 = item_pickup_new(gfc_vector2d(800, 1200), "pickup_drone", "images/toolbox_red.png", 32, 32);
    item4 = item_pickup_new(gfc_vector2d(900, 1200), "pickup_smoke", "images/toolbox_yellow.png", 32, 32);
    item5 = item_pickup_new(gfc_vector2d(1000, 1200), "pickup_jump", "images/toolbox_green.png", 32, 32);
    item6 = item_pickup_new(gfc_vector2d(2400, 1100), "pickup_diamond", "images/diamond.png", 32, 32);
    stalagmite = interactable_new(gfc_vector2d(1500, 1057), "good_stalagmite", "images/stalagmite.png", 32, 64);
    rope = interactable_new(gfc_vector2d(1900, 1056), "rope", "images/rope.png", 8, 64);
    grass = interactable_new(gfc_vector2d(3000, 1358), "grass", "images/grass.png", 128, 96);
    win = main_menu();
    obj = objectives_menu();
    obj->hidden = 1;
    gfc_line_cpy(obj->name, "objectives_menu");
    mouse = gf2d_sprite_load_all("images/pointer.png",32,32,16,0);
    slog("press [escape] to quit");

    /*main game loop*/
    while(!done)
    {
        /*update things here*/
        gfc_input_update(); // inputs/controls

        font_cleanup(); // cleanup the font cache

        SDL_GetMouseState(&mx, &my); // mouse
        mf += 0.1;
        if (mf >= 16.0) mf = 0;

        // update physics delta before calling entity think/update functions
        physics_update_delta();

        // player information
        if (game) entity_system_think();
		if (game) entity_system_update();

        // window information
        window_system_update();
        
        gf2d_graphics_clear_screen(); // clears drawing buffers
        // all drawing should happen betweem clear_screen and next_frame
    
            //backgrounds drawn first
            if (game) world_draw(world);

            //entities drawn next
            if (game) entity_system_draw();

            //UI elements last
            window_system_draw();

            gf2d_sprite_draw(
                mouse,
                gfc_vector2d(mx,my),
                NULL,
                NULL,
                NULL,
                NULL,
                &mouseGFC_Color,
                (int)mf);

        gf2d_graphics_next_frame(); // render current draw frame and skip to the next frame
        if (game)
        {
            if (gfc_input_key_pressed("m"))
            {
                if (obj->hidden) obj->hidden = 0;
                else obj->hidden = 1;
            }
        }
        if (gfc_input_key_pressed("ESCAPE")) done = 1;
        //slog("Rendering at %f FPS",gf2d_graphics_get_frames_per_second());
    }
    world_free(world);
    slog("---==== END ====---");
    return 0;
}

void start_game()
{
    game = 1;
}

void exit_game()
{
    done = 1;
}

/*eol@eof*/
