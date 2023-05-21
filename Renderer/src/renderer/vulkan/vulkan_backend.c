#include "vulkan_backend.h"
#include "vulkan_types.inl"
#include "core/logger.h"

#include "core/vstring.h"
#include "containers/darray.h"
#include "platform/platform.h"
#include "vulkan_platform.h"

// static vulkan context
static vulkan_context context;

#if defined(VKR_DEBUG)
VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_type,
    const VkDebugUtilsMessengerCallbackDataEXT* pcallback_data,
    void* puser_data);
#endif

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


    // Get extensions
    const char** required_extensions = darray_create(const char*);
    const char* surface_extension = VK_KHR_SURFACE_EXTENSION_NAME;
    darray_push(required_extensions, surface_extension); // Generic surface extensions
    platform_get_required_extensions_names(&required_extensions); // OS specific extensions

#if defined(VKR_DEBUG)
    const char* debug_ext = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
    darray_push(required_extensions, debug_ext);

    VDEBUG("Required extensions list:");
    u32 length = (u32)darray_length(required_extensions);
    for (u32 idx = 0; idx != length; ++idx) {
        VDEBUG(required_extensions[idx]);
    }
#endif

    // Get validation layers
    const char** required_validation_layer_names = 0;
    u32 required_validation_layers_count = 0;

// Enable validation layers
#if defined(VKR_DEBUG)
    VDEBUG("Validation layers enabled. Enumerating...");

    // Create the list of required validation layers
    required_validation_layer_names = darray_create(const char*);
    const char* KHR_validation_layer = "VK_LAYER_KHRONOS_validation";
    darray_push(required_validation_layer_names, KHR_validation_layer);
    required_validation_layers_count = (u32)darray_length(required_validation_layer_names);

    // Find the available validation layers
    u32 available_validation_layers_count = 0;
    vkEnumerateInstanceLayerProperties(&available_validation_layers_count, 0);
    VkLayerProperties* available_validation_layers_names = darray_reserve(VkLayerProperties, available_validation_layers_count);
    vkEnumerateInstanceLayerProperties(&available_validation_layers_count, available_validation_layers_names);

    for (u32 idx = 0; idx != required_validation_layers_count; ++idx) {
        VDEBUG("Searching for layer: %s...", required_validation_layer_names[idx]);
        b8 found = FALSE;
        for (u32 j = 0; j < available_validation_layers_count; ++j) {
            if (strings_equal(available_validation_layers_names[j].layerName, required_validation_layer_names[idx])) {
                found = TRUE;
                VDEBUG("Found");
                break;
            }
        }

        if (!found) {
            VFATAL("Required validation layer not found: %s", required_validation_layer_names[idx]);
            return FALSE;
        }
    }

    VDEBUG("All required validation layers were found");
#endif

    // Set up the instance information
    VkInstanceCreateInfo instance_info = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    instance_info.pApplicationInfo = &app_info;
    instance_info.enabledExtensionCount = (u32)darray_length(required_extensions);
    instance_info.ppEnabledExtensionNames = required_extensions;
    instance_info.enabledLayerCount = required_validation_layers_count;
    instance_info.ppEnabledLayerNames = required_validation_layer_names;
    instance_info.flags = 0;
    instance_info.pNext = 0;

    VkResult result = vkCreateInstance(&instance_info, context.allocator, &context.instance);
    VK_CHECK(result);
    VINFO("Vulkan instance created");

    // Create vulkan debugger
#if defined(VKR_DEBUG)

    // Create structure information and fill it
    VkDebugUtilsMessengerCreateInfoEXT debugger_info;
    debugger_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugger_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    //VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
    //     VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT; extra verbosity of needed
    debugger_info.flags = 0;
    debugger_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debugger_info.pUserData = 0; // HACK: probably not needed for now
    debugger_info.pfnUserCallback = debugCallback;
    debugger_info.pNext = 0;

    // Load the extension function
    PFN_vkCreateDebugUtilsMessengerEXT func;
    func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(context.instance, "vkCreateDebugUtilsMessengerEXT");
    VASSERT_MSG(func, "Failed to load function pointer: vkCreateDebugUtilsMessengerEXT");
    
    // Create the debug messenger
    VkResult result_debugger = func(context.instance, &debugger_info, context.allocator, &context.debugger);
    VK_CHECK(result_debugger);
    VDEBUG("Vulkan debug mesenger created");
#endif

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

#if defined(VKR_DEBUG)
VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_type,
    const VkDebugUtilsMessengerCallbackDataEXT* pcallback_data,
    void* puser_data) {

    // Could access more info on the message if needed
    switch (message_severity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        VERROR(pcallback_data->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        VINFO(pcallback_data->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        VWARN(pcallback_data->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        VFATAL(pcallback_data->pMessage);
        break;
    }

    return FALSE;
}
#endif