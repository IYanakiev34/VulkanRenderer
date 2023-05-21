#include <entry.h>
#include <core/vmemory.h>
#include "game.h"

b8 create_game(game* game_out) {
    if (!game_out)
        return FALSE;

    // Application configuration
    {
        game_out->app_config.name = "Testbed Game";
        game_out->app_config.start_x = 100;
        game_out->app_config.start_y = 100;
        game_out->app_config.start_width = 1200;
        game_out->app_config.start_height = 720;
    }

    // Assign function pointers
    {
        game_out->initialize = game_initialize;
        game_out->render = game_render;
        game_out->update = game_update;
        game_out->on_resize = game_on_resize;
    }
    
    // Assign game state
    {
        game_out->state = vallocate(sizeof(game_state),MEMORY_TAG_GAME);
    }

    return TRUE;
}