#include "vulkan_backend.h"
#include "vulkan_types.inl"
#include "core/logger.h"

// static vulkan context
static vulkan_context context;

b8 vulkan_renderer_backend_initialize(renderer_backend* backend, const char* application_name, struct platform_state* plat_state) {
    // TODO: custom allocator
    context.allocator = 0;

    // Setup Vulkan application information
    VkApplicationInfo app_info = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
    app_info.apiVersion = VK_API_VERSION_1_3;
    app_info.pApplicationName = application_name;
    app_info.pEngineName = "Tiny Engine";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);

    // Set up the instance information
    VkInstanceCreateInfo instance_info = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    instance_info.enabledExtensionCount = 0; // ?
    instance_info.enabledLayerCount = 0; // ?
    instance_info.pApplicationInfo = &app_info;
    instance_info.ppEnabledExtensionNames = 0; // no idea
    instance_info.ppEnabledLayerNames = 0;// no idea;

    VkResult result = vkCreateInstance(&instance_info, context.allocator, &context.instance);
    if (result != VK_SUCCESS) {
        VERROR("vkCreateInstance failed with result: %u", result);
        return FALSE;
    }

    VINFO("Vulkan renderer intialized successfully.");
    return TRUE;
}

void vulkan_renderer_backend_shutdown(renderer_backend* backend) {
    // TODO: clean up
}

void vulkan_renderer_backend_resized(renderer_backend* backend, u16 width, u16 height) {
    // TODO: handle resized
}

b8 vulkan_renderer_backend_begin_frame(renderer_backend* backend, f64 delta_time) {
    return TRUE;
}

b8 vulkan_renderer_backend_end_frame(renderer_backend* backend, f64 delta_time) {
    return TRUE;
}