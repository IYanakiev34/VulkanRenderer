#pragma once
#include "renderer_types.inl"

// Future 
struct static_mesh_data;
///////////////////////////

struct platform_state;

/**
* Initializing the renderer backend.
* 
* @param application_name - The name of the application (comes from engine)
* @param plat_state - The current platform state (comes from app_state in application.c)
* 
* @return b8 - TRUE if successfully initialized, FALSE otherwise
*/
b8 renderer_initialize(const char* application_name, struct platform_state* plat_state);

/**
* Shutdowns the renderer backend and cleans up all necessary resources
*/
void renderer_shutdown();

/**
* Draws the current frame using the renderer backend.
* 
* @param packet - The information needed to draw the frame
* @return b8 - TRUE if the frame was renderer successfully, FALSE otherwise
*/
b8 renderer_draw_frame(render_packet* packet);

/**
* Handles window resize if the platform has the concept of a window.
* 
* @param width - The new width of the screen / window
* @param height - The new height of the screen / window
*/
void renderer_on_resize(u16 width, u16 height);