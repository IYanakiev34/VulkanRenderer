#pragma once
#include "renderer/renderer_backend.h"


b8 vulkan_renderer_backend_initialize(renderer_backend* backend, const char* application_name, struct platform_state* plat_state);
void vulkan_renderer_backend_shutdown (renderer_backend* backend);
void vulkan_renderer_backend_resized (renderer_backend* backend, u32 width, u32 height);
b8 vulkan_renderer_backend_begin_frame (renderer_backend* backend, f64 delta_time);
b8 vulkan_renderer_backend_end_frame (renderer_backend* backend, f64 delta_time);