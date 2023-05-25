#include "vulkan_backend.h"
#include "vulkan_types.inl"
#include "vulkan_device.h"
#include "vulkan_platform.h"
#include "vulkan_swapchain.h"
#include "vulkan_renderpass.h" 
#include "vulkan_command_buffer.h"
#include "vulkan_framebuffer.h"
#include "vulkan_fence.h"
#include "vulkan_utils.h"

// General includes
#include "core/logger.h"
#include "core/vstring.h"
#include "core/vmemory.h"
#include "containers/darray.h"
#include "platform/platform.h"
#include "core/application.h"

// static vulkan context
static vulkan_context context;
static u32 cached_framebuffer_width = 0;
static u32 cached_framebuffer_height = 0;

#if defined(VKR_DEBUG)
VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_type,
    const VkDebugUtilsMessengerCallbackDataEXT* pcallback_data,
    void* puser_data);
#endif

i32 find_memory_index(u32 type_filter, u32 property_flags);

void create_command_buffers(renderer_backend* backend);
void regenerate_framebuffers(renderer_backend* backend, vulkan_swapchain* swapchain, vulkan_renderpass* renderpass);
b8 recreate_swapchain(renderer_backend* backend);

b8 vulkan_renderer_backend_initialize(renderer_backend* backend, const char* application_name, struct platform_state* plat_state) {
    context.find_memory_index = find_memory_index;

    // TODO: custom allocator
    context.allocator = 0;

    application_get_framebuffer_size(&cached_framebuffer_width, &cached_framebuffer_height);
    context.framebuffer_width = (cached_framebuffer_width != 0) ? cached_framebuffer_width : 1200;
    context.framebuffer_height = (cached_framebuffer_height != 0) ? cached_framebuffer_height : 720;
    cached_framebuffer_width = 0;
    cached_framebuffer_height = 0;

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
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT; //extra verbosity of needed
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
    // Destroy darray resources extension names etc
    {
        darray_destroy(required_validation_layer_names);
        darray_destroy(required_extensions);
#if defined(VKR_DEBUG)
        darray_destroy(available_validation_layers_names);
#endif
    }

    // Surface creation
    VINFO("Creating vulkan surface...")
    if (!platform_create_vulkan_surface(backend->plat_state, &context)) {
        VERROR("Could not create vulkan surface. Aborting initalization of backend");
        return FALSE;
    }
    VINFO("Vulkan surface created!");

    // Device creation
    if (!vulkan_device_create(&context)) {
        VFATAL("Failed to create vulkan device!");
        return FALSE;
    }
    VINFO("Logical Device created!");

    // Swapchain creation
    VINFO("Creating swapchain...");
    vulkan_swapchain_create(&context, context.framebuffer_width, context.framebuffer_height, &context.swapchain);

    // Create renderpass TODO: should probably fix it
    VINFO("Creating main render pass...");
    vulkan_renderpass_create(
        &context,
        &context.main_renderpass,
        0,0, context.framebuffer_width,context.framebuffer_height,
        0.f,0.f,0.2f,1.f, // Clear Color
        1.0f,
        0);
    VINFO("Main vulkan renderpass created!");

    // Create swapchain buffers
    context.swapchain.framebuffers = (vulkan_framebuffer *)darray_reserve(vulkan_framebuffer, context.swapchain.image_count);
    regenerate_framebuffers(backend, &context.swapchain, &context.main_renderpass);

    // Create command buffers
    create_command_buffers(backend);

    // Create sync objects
    context.image_available_semaphores = darray_reserve(VkSemaphore, context.swapchain.max_frames_in_flight);
    context.queue_complete_semaphore = darray_reserve(VkSemaphore, context.swapchain.max_frames_in_flight);
    context.in_flight_fences = darray_reserve(vulkan_fence, context.swapchain.max_frames_in_flight);

    for (u8 idx = 0; idx != context.swapchain.max_frames_in_flight; ++idx) {
        VkSemaphoreCreateInfo semaphore_info = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
        VkResult res = vkCreateSemaphore(context.device.logical_device, &semaphore_info, context.allocator, &context.image_available_semaphores[idx]);
        VK_CHECK(res);
        res = vkCreateSemaphore(context.device.logical_device, &semaphore_info, context.allocator, &context.queue_complete_semaphore[idx]);
        VK_CHECK(res);

        // Create the fence in signaled state, indicating that the first frame has already been  "renderer"
        // This will prevent the application from infinitely waiting for a frame to be rendered and to signal
        // so. Since a frame cannot be rendered since a frame before it has been renderer
        vulkan_fence_create(&context, TRUE, &context.in_flight_fences[idx]);
    }
    
    // In flight fences should not exists until this point. Orignal pointers should be 0
    // Actual owned fences are the in_flight_fences we point to them via an index
    // of the current frame so we know which fence is for the current frame
    context.images_in_flight = darray_reserve(vulkan_fence, context.swapchain.image_count);
    for (u32 idx = 0; idx != context.swapchain.image_count; ++idx)
        context.images_in_flight[idx].handle = 0;

    VINFO("Vulkan renderer intialized successfully.");
    return TRUE;
}

void vulkan_renderer_backend_shutdown(renderer_backend* backend) {
    VkResult res = vkDeviceWaitIdle(context.device.logical_device);
    VK_CHECK(res);

    // Sync objects
    VINFO("Destroying synchronization objects...");
    for (u8 idx = 0; idx != context.swapchain.max_frames_in_flight; ++idx) {
        if (context.image_available_semaphores[idx]) {
            vkDestroySemaphore(context.device.logical_device, context.image_available_semaphores[idx], context.allocator);
        }
        if (context.queue_complete_semaphore[idx]) {
            vkDestroySemaphore(context.device.logical_device, context.queue_complete_semaphore[idx], context.allocator);
        }
        vulkan_fence_destroy(&context, &context.in_flight_fences[idx]);
    }
    darray_destroy(context.image_available_semaphores);
    context.image_available_semaphores = 0;

    darray_destroy(context.queue_complete_semaphore);
    context.queue_complete_semaphore = 0;
    darray_destroy(context.in_flight_fences);
    context.in_flight_fences = 0;

    darray_destroy(context.images_in_flight);
    context.images_in_flight = 0;
    VINFO("Destroyed synchronization objects!");

    // Destroy command buffers
    VINFO("Destroying command buffers...");
    for (u32 idx = 0; idx != context.swapchain.image_count; ++idx) {
        if (context.graphics_command_buffers->handle) {
            vulkan_command_buffer_free(&context, context.device.graphics_command_pool, &context.graphics_command_buffers[idx]);
            context.graphics_command_buffers->handle = 0;
        }
    }
    darray_destroy(context.graphics_command_buffers);
    context.graphics_command_buffers = 0;
    VINFO("Command buffers destroyed\n");

    VINFO("Destroying main renderpass...");
    vulkan_renderpass_destroy(&context, &context.main_renderpass);
    VINFO("Destroyed main renderpass!\n");

    VINFO("Destroying framebuffers of swapchain...");
    for (u32 idx = 0; idx != context.swapchain.image_count; ++idx)
        vulkan_framebuffer_destroy(&context, &context.swapchain.framebuffers[idx]);
    darray_destroy(context.swapchain.framebuffers);
    VINFO("Destroyed swapchain framebuffers!");

    // Swapchain
    VINFO("Destroying swapchain...")
    vulkan_swapchain_destroy(&context, &context.swapchain);
    VINFO("Destroyed swapchain!\n");

    // Destroy debugger
#if defined(VKR_DEBUG)
    VDEBUG("Destroying Vulkan Debugger...");
    PFN_vkDestroyDebugUtilsMessengerEXT destroy_func;
    destroy_func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(context.instance, "vkDestroyDebugUtilsMessengerEXT");
    VASSERT(destroy_func);
    destroy_func(context.instance, context.debugger, context.allocator);
    VDEBUG("Destroyed vulkan debugger!\n");
#endif

    // Destroy surface
    VINFO("Destroying surface...");
    if (context.surface)
        vkDestroySurfaceKHR(context.instance, context.surface, context.allocator);
    VINFO("Destroyed surface!\n");

    VINFO("Destroying device...");
    vulkan_device_destroy(&context);
    VINFO("Destroyed device!\n");

    // Destroy instance
    VINFO("Destroying Vulkan Instance...");
    vkDestroyInstance(context.instance, context.allocator);
    VINFO("Destroyed instance\n");
}

void vulkan_renderer_backend_resized(renderer_backend* backend, u32 width, u32 height) {
    context.framebuffer_width = width;
    context.framebuffer_height = height;
    context.framebuffer_size_generation++;
    cached_framebuffer_width = context.framebuffer_width;
    cached_framebuffer_height = context.framebuffer_height;

    VINFO("Vulkan renderer backed->resized w/h/gen: %i/%i/%llu", width, height, context.framebuffer_size_generation);
}

b8 vulkan_renderer_backend_begin_frame(renderer_backend* backend, f64 delta_time) {
    vulkan_device* device = &context.device;

    // Check if we are in the middle of swapchain recreation
    if (context.recreating_swapchain) {
        VkResult res = vkDeviceWaitIdle(device->logical_device);
        if (!vulkan_result_is_success(res)) {
            VERROR("vulkan_renderer_backend_begin_frame -> vkDeviceWaitIdle");
            return FALSE;
        }

        return FALSE;
    }

    // Check if we need to resize the framebuffer
    if (context.framebuffer_size_generation != context.framebuffer_size_last_generation) {
        VkResult res = vkDeviceWaitIdle(device->logical_device);
        if (!vulkan_result_is_success(res)) {
            VERROR("vulkan_renderer_backend_begin_frame -> vkDeviceWaitIdle when checking framebuffer size");
            return FALSE;
        }

        if (!recreate_swapchain(backend))
            return FALSE;

        VINFO("Resized. Booting");
        return FALSE;
    }

    if (!vulkan_fence_wait(&context, &context.in_flight_fences[context.current_frame], UINT64_MAX)) {
        VWARN("In-flight fences wait failure");
        return FALSE;
    }

    if (!vulkan_swapchain_acquire_next_image_index(
        &context,&context.swapchain,
        UINT64_MAX, context.image_available_semaphores[context.current_frame],
        0, &context.image_index)) {
        return FALSE;
    }

    // Begin command recording
    vulkan_command_buffer* command_buffer = &context.graphics_command_buffers[context.image_index];
    vulkan_command_buffer_reset(command_buffer);
    vulkan_command_buffer_begin_recording(command_buffer, FALSE, FALSE, FALSE);

    // Dynamic state
    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = (f32)context.framebuffer_height; // Vulkan starts at top left we want to make sure we always strat bottom left like OpenGL
    viewport.width = (f32)context.framebuffer_width;
    viewport.height = - (f32)context.framebuffer_height; // Render botton up
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    // Scissor
    VkRect2D scissor;
    scissor.offset.x = scissor.offset.y = 0;
    scissor.extent.width = context.framebuffer_width;
    scissor.extent.height = context.framebuffer_height;

    // Set view port and clipping space
    vkCmdSetViewport(command_buffer->handle, 0, 1, &viewport);
    vkCmdSetScissor(command_buffer->handle, 0, 1, &scissor);

    // Begin renderpass
    context.main_renderpass.w = (f32)context.framebuffer_width;
    context.main_renderpass.h = (f32)context.framebuffer_height;
    vulkan_renderpass_begin(command_buffer, &context.main_renderpass, context.swapchain.framebuffers[context.image_index].handle);

    return TRUE;
}

b8 vulkan_renderer_backend_end_frame(renderer_backend* backend, f64 delta_time) {
    vulkan_command_buffer* command_buffer = &context.graphics_command_buffers[context.image_index];
    vulkan_renderpass_end(command_buffer, &context.main_renderpass);
    vulkan_command_buffer_end_recording(command_buffer);

    // Make sure previous frame is not using this imaghe (i.e it's fence is being waited on)
    if (context.images_in_flight[context.image_index].handle != VK_NULL_HANDLE)
        vulkan_fence_wait(&context, &context.in_flight_fences[context.image_index], UINT64_MAX);

    // Make image fence as in-use by this frame TODO: fix it
    // context.images_in_flight[context.image_index] = context.in_flight_fences[context.current_frame];
    

    // Reset the fence
    vulkan_fence_reset(&context, &context.in_flight_fences[context.current_frame]);

    VkSubmitInfo submit_info = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
    submit_info.pNext = 0;
    // Buffers to execute
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer->handle;
    
    // Semaphores to be signaled on queue completion
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &context.queue_complete_semaphore[context.current_frame];

    // Wait semaphores to ensure the operation cannot begin until the image is available
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &context.image_available_semaphores[context.current_frame];

    // each semaphore waits on the corresponding pipeline stage to complete: 1 to 1 ratio.
    // VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT prevents subsequent color attachments
    // writes from executing unitl semaphore signals (i.e one frame is presented at a time)
    VkPipelineStageFlags flags[1] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submit_info.pWaitDstStageMask = flags;

    VkResult res = vkQueueSubmit(
        context.device.graphics_queue,
        1,
        &submit_info,
        context.in_flight_fences[context.current_frame].handle);

    if (res != VK_SUCCESS) {
        VERROR("vkQueueSubmit failed with resutl: %s", vulkan_result_string(res, TRUE));
        return FALSE;
    }

    // Update state of the comamnd buffer
    vulkan_command_buffer_update_submit(command_buffer);

    // Present stage (give image back to the swapchain
    vulkan_swapchain_present(
        &context,
        &context.swapchain,
        context.device.graphics_queue,
        context.device.present_queue,
        context.queue_complete_semaphore[context.current_frame],
        context.image_index);

    return TRUE;
}

i32 find_memory_index(u32 type_filter, u32 property_flags) {
    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(context.device.physical_device, &memory_properties);

    for (u32 idx = 0; idx != memory_properties.memoryTypeCount; ++idx) {
        if (type_filter & (1 << idx) && (memory_properties.memoryTypes[idx].propertyFlags & property_flags) == property_flags)
            return idx;
    }

    VWARN("Unable to find suitable memory type");
    return -1;
}

void create_command_buffers(renderer_backend* backend) {
    if (!context.graphics_command_buffers) {
        context.graphics_command_buffers = darray_reserve(vulkan_command_buffer, context.swapchain.image_count);
        for (u32 idx = 0; idx != context.swapchain.image_count; ++idx)
            vzero_memory(&context.graphics_command_buffers[idx], sizeof(vulkan_command_buffer));   
    }

    for (u32 idx = 0; idx != context.swapchain.image_count; ++idx) {
        if (context.graphics_command_buffers[idx].handle)
            vulkan_command_buffer_free(&context, context.device.graphics_command_pool, &context.graphics_command_buffers[idx]);

        vzero_memory(&context.graphics_command_buffers[idx], sizeof(vulkan_command_buffer));
        vulkan_command_buffer_allocate(&context, 
            context.device.graphics_command_pool, 
            TRUE, 
            &context.graphics_command_buffers[idx]);
    }
    VINFO("Vulkan command buffers created");
}

void regenerate_framebuffers(renderer_backend* backend, vulkan_swapchain* swapchain, vulkan_renderpass* renderpass) {
    for (u32 idx = 0; idx != swapchain->image_count; ++idx) {
        // TODO: dynamic based on currently configured attachments
        u32 attachment_count = 2;
        VkImageView attachments[] = {
            swapchain->views[idx],
            swapchain->depth_attachment.view
        };

        vulkan_framebuffer_create(
            &context,
            renderpass,
            context.framebuffer_width,
            context.framebuffer_height,
            attachment_count,
            attachments,
            &swapchain->framebuffers[idx]
        );
    }
}

b8 recreate_swapchain(renderer_backend* backend) {
    // If already recreating swapchain
    if (context.recreating_swapchain) {
        VDEBUG("recreate_swapchain called when already recreating. Booting");
        return FALSE;
    }

    if (context.framebuffer_width == 0 || context.framebuffer_height == 0) {
        VDEBUG("recreate_swapchain called when window is < 1 in dimensions. Booting");
        return FALSE;
    }

    // Mark recreating
    context.recreating_swapchain = TRUE;
    VkResult res = vkDeviceWaitIdle(context.device.logical_device);
    VK_CHECK(res);

    // Clear out just in case
    for (u32 idx = 0; idx != context.swapchain.image_count; ++idx)
        context.images_in_flight[idx].handle = 0;


    // Destroy Old FrameBuffers
    for (u32 idx = 0; idx != context.swapchain.image_count; ++idx)
        vulkan_framebuffer_destroy(&context, &context.swapchain.framebuffers[idx]);

    // Recreate
    vulkan_swapchain_recreate(&context, cached_framebuffer_width, cached_framebuffer_height, &context.swapchain);

    // Sync size
    context.framebuffer_width = cached_framebuffer_width;
    context.framebuffer_height = cached_framebuffer_height;
    context.main_renderpass.w = (f32)context.framebuffer_width;
    context.main_renderpass.h = (f32)context.framebuffer_height;
    cached_framebuffer_width = 0;
    cached_framebuffer_height = 0;

    // update framebuffer size generation
    context.framebuffer_size_last_generation = context.framebuffer_size_generation;


    context.main_renderpass.x = 0;
    context.main_renderpass.y = 0;
    context.main_renderpass.w = (f32)context.framebuffer_width;
    context.main_renderpass.h = (f32)context.framebuffer_height;

    // Create new framebuffers + command buffers for the swapchain
    regenerate_framebuffers(backend, &context.swapchain, &context.main_renderpass);
    create_command_buffers(backend);

    // Flip flag
    context.recreating_swapchain = FALSE;

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