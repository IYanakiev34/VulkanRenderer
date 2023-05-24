#pragma once

#include "defines.h"

typedef struct application_config {
    // Position
    i32 start_x;
    i32 start_y;

    // Dimensions
    i32 start_width;
    i32 start_height;
    
    // Name
    const char* name;
} application_config;

VAPI b8 application_create(struct game* game_inst);

VAPI b8 application_run();

void application_get_framebuffer_size(u32* width, u32* height);
