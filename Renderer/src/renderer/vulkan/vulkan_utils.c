#include "vulkan_utils.h"

b8 vulkan_result_is_success(VkResult res) {
    switch (res) {
    case VK_SUCCESS:
    case VK_NOT_READY:
    case VK_TIMEOUT:
    case VK_EVENT_SET:
    case VK_EVENT_RESET:
    case VK_SUBOPTIMAL_KHR:
    case VK_THREAD_IDLE_KHR:
    case VK_THREAD_DONE_KHR:
    case VK_OPERATION_DEFERRED_KHR:
    case VK_OPERATION_NOT_DEFERRED_KHR:
    case VK_PIPELINE_COMPILE_REQUIRED_EXT:
        return TRUE;
    case VK_ERROR_OUT_OF_HOST_MEMORY:
    case VK_ERROR_DEVICE_LOST:
    case VK_ERROR_OUT_OF_DEVICE_MEMORY:
    case VK_ERROR_OUT_OF_POOL_MEMORY_KHR:
    case VK_ERROR_INITIALIZATION_FAILED:
    case VK_ERROR_MEMORY_MAP_FAILED:
    case VK_ERROR_LAYER_NOT_PRESENT:
        //TODO: add more
        return FALSE;
    }

    return TRUE;
}


const char* vulkan_result_string(VkResult res, b8 get_extended) {
    // TODO:

    return 0;
}