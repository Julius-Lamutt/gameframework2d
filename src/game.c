#include <SDL.h>
#include "simple_logger.h"
#include "gfc_input.h"
#include "gf2d_graphics.h"
#include "gf2d_sprite.h"
#include "font.h"
#include "windows.h"
#include "camera.h"
#include "entity.h"
#include "items.h"
#include "player.h"
#include "bullet.h"
#include "main_menu.h"
#include "world.h"

/*global variables & flags*/
const Bool f_collision_draw = false; // for collision debugging

int main(int argc, char *argv[])
{
    /*variable declarations*/
    int mx,my, done = 0;
    float mf = 0;
    World *world;
    Window *win;
    Entity *player;
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
    window_system_init(128);
	entity_system_init(1024);
    camera_set_size(gfc_vector2d(1200, 720));
    gfc_input_init("defs/config.json");
    items_init("defs/items.json");

    SDL_ShowCursor(SDL_DISABLE);
    
    /*demo setup*/
    world = world_load("defs/maps/testworld.json");
    world_setup_camera(world);
    player = player_new();
    player->world = world;
    win = main_menu();
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

        // player information
        entity_system_think();
		entity_system_update();

        window_system_update();
        
        gf2d_graphics_clear_screen(); // clears drawing buffers
        // all drawing should happen betweem clear_screen and next_frame
    
            //backgrounds drawn first
            world_draw(world);

            //entities drawn next
            entity_system_draw();

            //UI elements last
            window_system_draw();

            font_draw_text("Press esc to quit\nain't that neat?", FS_MEDIUM, GFC_COLOR_BLACK, gfc_vector2d(10, 10));

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
        
        if (gfc_input_key_pressed("ESCAPE")) done = 1;
    }
    world_free(world);
    slog("---==== END ====---");
    return 0;
}
/*eol@eof*/
