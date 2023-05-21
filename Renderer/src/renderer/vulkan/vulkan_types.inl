#pragma once
#include "defines.h"
#include "core/vassert.h"
#include <vulkan/vulkan.h>

typedef struct vulkan_context {
    VkInstance instance;
    VkAllocationCallbacks* allocator;
#if defined (VKR_DEBUG)
    VkDebugUtilsMessengerEXT debugger;
#endif
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