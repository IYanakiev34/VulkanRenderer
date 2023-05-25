#include "renderer_frontend.h"
#include "renderer_backend.h"
#include "core/vmemory.h"
#include "core/logger.h"


// Backend render context
static renderer_backend* backend = 0;

b8 renderer_initialize(const char* application_name, struct platform_state* plat_state) {
    backend = vallocate(sizeof(renderer_backend), MEMORY_TAG_RENDERER);

    // TODO: type should be configurable
    renderer_backend_create(RENDERER_BACKEND_VULKAN, plat_state, backend);

    if (!backend->initialize(backend, application_name, plat_state))
        return FALSE;

    return TRUE;
}

void renderer_shutdown() {
    backend->shutdown(backend);
    renderer_backend_destroy(backend);
    vfree(backend, sizeof(renderer_backend), MEMORY_TAG_RENDERER);
}

b8 renderer_begin_frame(f64 delta_time) {
    return backend->begin_frame(backend, delta_time);
}

b8 renderer_end_frame(f64 delta_time) {
    b8 result = backend->end_frame(backend, delta_time);
    ++backend->frame_count;
    return result;
}

b8 renderer_draw_frame(render_packet* packet) {
    if (renderer_begin_frame(packet->delta_time)) {
        b8 result = renderer_end_frame(packet->delta_time);
        if (!result) {
            VFATAL("renderer_end_frame failed. Application shutting down...");
            return FALSE;
        }
    }

    return TRUE;
}

void renderer_on_resize(u16 width, u16 height) {
    if (backend) {
        backend->resized(backend, (u32) width, (u32)height);
    }
    else {
        VERROR("renderer_on_resize - > Backend does not exist!");
    }
}
