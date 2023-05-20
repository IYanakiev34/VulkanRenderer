#include "renderer_backend.h"
#include "vulkan/vulkan_backend.h"
#include "core/logger.h"

b8 renderer_backend_create(renderer_backend_type backend_type, struct platform_state* plat_state, renderer_backend* out_backend) {
    out_backend->plat_state = plat_state;

    switch (backend_type) {
    case RENDERER_BACKEND_VULKAN:
        out_backend->initialize = vulkan_renderer_backend_initialize;
        out_backend->shutdown = vulkan_renderer_backend_shutdown;
        out_backend->begin_frame = vulkan_renderer_backend_begin_frame;
        out_backend->end_frame = vulkan_renderer_backend_end_frame;
        out_backend->resized = vulkan_renderer_backend_resized;
        return TRUE;
    case RENDERER_BACKEND_DIRECTX:
        VFATAL("DirectX is not supported currently");
        return FALSE;
    case RENDERER_BACKEND_OPENGL:
        VFATAL("OpenGL is not supported currently");
        return FALSE;
    }

    VFATAL("Unrecognized renderer backend type passed");
    return FALSE;
}

void renderer_backend_destroy(renderer_backend* backend) {
    backend->begin_frame = 0;
    backend->shutdown = 0;
    backend->end_frame = 0;
    backend->initialize = 0;
    backend->plat_state = 0;
    backend->resized = 0;
}