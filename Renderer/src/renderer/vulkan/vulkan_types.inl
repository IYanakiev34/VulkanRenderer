#pragma once
#include "defines.h"
#include "core/vassert.h"
#include <vulkan/vulkan.h>

typedef struct vulkan_swapchain_support_info {
    VkSurfaceCapabilitiesKHR capabilities;
    u32 format_count;
    VkSurfaceFormatKHR* formats;
    u32 present_mode_count;
    VkPresentModeKHR* present_mode;
} vulkan_swapchain_support_info;

typedef struct vulkan_device {
    VkPhysicalDevice physical_device;
    VkDevice logical_device;
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties memory;

    // Queue indices
    u8 graphics_queue_index;
    u8 transfer_queue_index;
    u8 present_queue_index;
    u8 compute_queue_index; // HACK: used for now

    // Queue handles
    VkQueue graphics_queue;
    VkQueue transfer_queue;
    VkQueue present_queue;
    // TODO: add the compute queue when needed

    vulkan_swapchain_support_info swapchain_support;
} vulkan_device;

typedef struct vulkan_context {
    VkInstance instance;
    VkAllocationCallbacks* allocator;
    VkSurfaceKHR surface;

#if defined (VKR_DEBUG)
    VkDebugUtilsMessengerEXT debugger;
#endif
    vulkan_device device;
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