#pragma once
#include "defines.h"
#include "core/vassert.h"
#include <vulkan/vulkan.h>

/*
* Information about the swapchain.
* What types and formats it supports, what present modes it has
* and how many there are.
*/
typedef struct vulkan_swapchain_support_info {
    VkSurfaceCapabilitiesKHR capabilities;
    u32 format_count;
    VkSurfaceFormatKHR* formats;
    u32 present_mode_count;
    VkPresentModeKHR* present_mode;
} vulkan_swapchain_support_info;

/*
* Encapsulates the physical and logical vulkan device.
* We store also the properties, features, and memory
* of the physical gpu for potential use.
* On the logical device side we keep handles to it
* and it's assocaited queues. We also maintain
* an index of the queue families. Finally a device
* should know about it's presentation capabilities, AKA
* the swapchain info.
*/
typedef struct vulkan_device {
    VkPhysicalDevice physical_device;
    VkDevice logical_device;
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties memory;

    // Formats
    VkFormat depth_format;

    // Queue family indices
    u8 graphics_queue_index;
    u8 transfer_queue_index;
    u8 present_queue_index;
    u8 compute_queue_index; // HACK: used for now

    // Queue handles
    VkQueue graphics_queue;
    VkQueue transfer_queue;
    VkQueue present_queue;
    // TODO: add the compute queue when needed

    VkCommandPool graphics_command_pool;

    vulkan_swapchain_support_info swapchain_support;
} vulkan_device;

/*
* Representation of a Vulkan Image. At minimal
* we need a handel to the image, a view to the image
* and a memory associated to the image. We also
* store the width and height for convenience if needed.
*/
typedef struct vulkan_image {
    VkImage handle;
    VkDeviceMemory memory;
    VkImageView view;
    u32 width;
    u32 height;
} vulkan_image;



// Renderpass states
typedef enum vulkan_renderpass_state {
    READY,
    RECORDING,
    IN_RENDER_PASS,
    RECORDING_ENDED,
    SUBMITED,
    NOT_ALLOCATED,
} vulkan_renderpass_state;

/*
* Representation of a vulkan renderpass. Store the handle to the pass
* along with information about the position and area on which it operates.
* Of course we also store information about the depth and stencil since
* the pass might involve depth or stencil operations.
* 
* We also store the state so that we know what the pass is currently doing
*/
typedef struct vulkan_renderpass {
    VkRenderPass handle;
    f32 x, y, w, h;
    f32 r, g, b, a;
    f32 depth;
    u32 stencil;

    vulkan_renderpass_state state;
} vulkan_renderpass;

typedef struct vulkan_framebuffer {
    VkFramebuffer handle;
    u32 attachment_count;
    VkImageView* attachments;
    vulkan_renderpass* renderpass;
} vulkan_framebuffer;

/*
* Stores information about the swapchain and a handle to it
* so that we can use it. We need to know how many frames
* in flight we can render to and also store their assocaited
* images and views to access them.
*/
typedef struct vulkan_swapchain {
    VkSurfaceFormatKHR format;
    u8 max_frames_in_flight;
    VkSwapchainKHR handle;
    u32 image_count;
    VkImage* images;
    VkImageView* views;
    vulkan_image depth_attachment;

    // Framebuffers used for on-screen rendering
    vulkan_framebuffer* framebuffers;
} vulkan_swapchain;


// State of the command buffer
typedef enum vulkan_commmand_buffer_state {
    COMMAND_BUFFER_STATE_READY,
    COMMAND_BUFFER_STATE_RECORDING,
    COMMAND_BUFFER_STATE_IN_RENDER_PASS,
    COMMAND_BUFFER_STATE_RECORDING_ENDED,
    COMMAND_BUFFER_STATE_SUBMITTED,
    COMMAND_BUFFER_STATE_NOT_ALLOCATED
} vulkan_commmand_buffer_state;

typedef struct vulkan_command_buffer {
    VkCommandBuffer handle;

    vulkan_commmand_buffer_state state;
} vulkan_command_buffer;

// Used for CPU-GPU synchronization
typedef struct vulkan_fence {
    VkFence handle;
    b8 is_signaled;
} vulkan_fence;

/* Main data structure to store the context of vulkan
* with all of the needed handles. This context should be used
* throughout the application for most of the graphics operations.
* Its stores the instance of the API alongs with a potentially
* custom allocator, a handle to a surface, debugger,
* and the device information.
*/
typedef struct vulkan_context {
    // Essentials
    VkInstance instance;
    VkAllocationCallbacks* allocator;
    VkSurfaceKHR surface;
    vulkan_device device;

    // Surface dimensions
    u32 framebuffer_width;
    u32 framebuffer_height;

#if defined (VKR_DEBUG)
    VkDebugUtilsMessengerEXT debugger;
#endif
    
    // Rendering
    vulkan_swapchain swapchain;
    vulkan_renderpass main_renderpass;
    vulkan_command_buffer* graphics_command_buffers; // darray

    // Synchronization objects
    VkSemaphore* image_available_semaphores;
    VkSemaphore* queue_complete_semaphore;
    u32 in_flight_fence_count;
    vulkan_fence* in_flight_fences;
    // Holds pointers to fences which exist and are owened elsewhere
    vulkan_fence** images_in_flight;

    u32 image_index; // TODO: to use it
    u32 current_frame; // TODO: to use it

    b8 recreating_swapchain; // TODO: to use it

    i32(*find_memory_index)(u32 type_filter, u32 property_flags);
}vulkan_context;

#define VK_CHECK(expr)         \
    {                               \
        VASSERT(expr == VK_SUCCESS); \
    }

typedef VkBool32(VKAPI_CALL* PFN_vkDebugUtilsMessengerCallbackEXT)(
    VkDebugUtilsMessageSeverityFlagBitsEXT           messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT                  messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData);