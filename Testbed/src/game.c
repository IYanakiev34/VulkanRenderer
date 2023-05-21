#include "game.h"
#include <core/logger.h>

// Initialization code of the game
b8 game_initialize(game* game_inst) {
    VINFO("game_initialize was called!");
    return TRUE;
}

// Logic to update the game / simulation
b8 game_update(game* game_inst, f64 delta_time) {
    return TRUE;
}

// Logic to render the view of the game
b8 game_render(game* game_inst, f64 delta_time) {
    return TRUE;
}

// Logic to handle window resize is window is a concept of the platform
void game_on_resize(game* game_inst, i32 new_width, i32 new_height) {

}