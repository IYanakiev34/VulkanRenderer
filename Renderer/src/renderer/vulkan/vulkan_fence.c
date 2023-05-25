#include "vulkan_fence.h"
#include "core/logger.h"


void vulkan_fence_create(
    vulkan_context* context,
    b8 is_signaled,
    vulkan_fence* out_fence) {
    out_fence->is_signaled = is_signaled;

    VkFenceCreateInfo fence_info = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
    fence_info.pNext = 0;
    if (out_fence->is_signaled)
        fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkResult res = vkCreateFence(context->device.logical_device, &fence_info, context->allocator, &out_fence->handle);
    VK_CHECK(res);
}

void vulkan_fence_destroy(vulkan_context* context, vulkan_fence* fence) {
    if (fence->handle) {
        vkDestroyFence(context->device.logical_device, fence->handle, context->allocator);
        fence->handle = 0;
    }
    fence->is_signaled = FALSE;
}

b8 vulkan_fence_wait(vulkan_context* context, vulkan_fence* fence, u64 timeout_ms) {
    if (!fence->is_signaled) {
        // TODO: could be multiple fences not only one
        VkResult result = vkWaitForFences(context->device.logical_device, 1, &fence->handle, VK_TRUE, timeout_ms);
        switch (result) {
        case VK_SUCCESS:
            fence->is_signaled = TRUE;
            return TRUE;
        case VK_TIMEOUT:
            VWARN("vk_fence_wait - Timed out!");
            break;
        case VK_ERROR_DEVICE_LOST:
            VERROR("vk_fence_wait - VK_ERROR_DEVICE_LOST!");
            break;
        case VK_ERROR_OUT_OF_HOST_MEMORY:
            VERROR("vk_fence_wait - VK_ERROR_OUT_OF_HOST_MEMORY!");
            break;
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            VERROR("vk_fence_wait - VK_ERROR_OUT_OF_DEVICE_MEMORY!");
            break;
        }
    }
    else
    {
        return TRUE;
    }

    return FALSE;
}

void vulkan_fence_reset(vulkan_context* context, vulkan_fence* fence) {
    // HACK: very important to fix
    if (fence->is_signaled) {
        VkResult res = vkResetFences(context->device.logical_device, 1, &fence->handle);
        VK_CHECK(res);
        fence->is_signaled = FALSE;
    }
}