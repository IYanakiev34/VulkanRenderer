#pragma once
#include "core/application.h"

typedef struct game {
    // Configuration of the application
    application_config app_config;

    // Initialization code of the game
    b8      (*initialize)(struct game* game_inst);

    // Logic to update the game / simulation
    b8      (*update)(struct game* game_inst, f32 delta_time);

    // Logic to render the view of the game
    b8      (*render)(struct game* game_inst, f32 delta_time);

    // Logic to handle window resize is window is a concept of the platform
    void    (*on_resize)(struct game* game_inst, i32 new_width, i32 new_height);

    // Internal game state. Created and managed by the game
    void*   state;
} game;