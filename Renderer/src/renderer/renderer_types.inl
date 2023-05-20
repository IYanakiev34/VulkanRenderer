#pragma once
#include "defines.h"

typedef enum renderer_backend_type {
    RENDERER_BACKEND_VULKAN,
    RENDERER_BACKEND_OPENGL,
    RENDERER_BACKEND_DIRECTX
} renderer_backend_type;

// Interface to a renderer backend
typedef struct renderer_backend {
    struct platform_state* plat_state;
    u64 frame_count;

    b8(*initialize)(struct renderer_backend* backend, const char* application_name, struct platform_state* plat_state);
    void (*shutdown)(struct renderer_backend* backend);
    void (*resized)(struct renderer_backend* backend, u16 width, u16 height);
    b8(*begin_frame)(struct renderer_backend* backend, f64 delta_time);
    b8(*end_frame)(struct renderer_backend* backend, f64 delta_time);
} renderer_backend;

// TODO: Will eventually have many more things
/*
* This struct should hold all the info for rendering a frame like:
* camera information,
* meshes,
* shaders,
* textures,
* buffers,
* etc
*/
typedef struct render_packet {
    f64 delta_time;
} render_packet;