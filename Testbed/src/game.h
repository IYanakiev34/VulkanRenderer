#pragma once

#include <defines.h>
#include <game_types.h>

typedef struct game_state {
    f32 delta_time;
} game_state;

// Initialization code of the game
b8 game_initialize(game* game_inst);

// Logic to update the game / simulation
b8 game_update(game* game_inst, f32 delta_time);

// Logic to render the view of the game
b8 game_render(game* game_inst, f32 delta_time);

// Logic to handle window resize is window is a concept of the platform
void game_on_resize(game* game_inst, i32 new_width, i32 new_height);