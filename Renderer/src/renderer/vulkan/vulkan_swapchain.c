#include "vulkan_swapchain.h"
#include "vulkan_device.h"
#include "vulkan_image.h"
#include "vulkan_framebuffer.h"

#include "core/logger.h"
#include "core/vmemory.h"
#include "containers/darray.h"

void create(vulkan_context* context, u32 width, u32 height, vulkan_swapchain* out_swapchain);
void destroy(vulkan_context* context, vulkan_swapchain* swapchain);

void vulkan_swapchain_create(
    vulkan_context* context,
    u32 width,
    u32 height,
    vulkan_swapchain* out_swapchain){
    // Create swapchain
    create(context, width, height, out_swapchain);
    VINFO("Vulkan swapchain created!");
}

void vulkan_swapchain_recreate(
    vulkan_context* context,
    u32 width,
    u32 height,
    vulkan_swapchain* swapchain) {
    // Destroy the old swapchain and create a new one
    destroy(context, swapchain);
    create(context, width, height, swapchain);
}

void vulkan_swapchain_destroy(
    vulkan_context* context,
    vulkan_swapchain* swapchain) {
    // Destroy the swapchain
    destroy(context, swapchain);
}

b8 vulkan_swapchain_acquire_next_image_index(
    vulkan_context* context,
    vulkan_swapchain* swapchain,
    u64 timeout_ms,
    VkSemaphore image_available_semaphore,
    VkFence fence,
    u32* out_image_index) {

    VkResult res = vkAcquireNextImageKHR(context->device.logical_device, swapchain->handle, timeout_ms, image_available_semaphore, fence, out_image_index);
    if (res == VK_ERROR_OUT_OF_DATE_KHR ) {
        // Recreate swapcahin new window size
        vulkan_swapchain_recreate(context, context->framebuffer_width, context->framebuffer_height, swapchain);
        return FALSE;
    }
    else if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR){
        VFATAL("Failed to acquire swapchain image!");
        return FALSE;
    }
    return TRUE;
}

void vulkan_swapchain_present(
    vulkan_context* context,
    vulkan_swapchain* swapchain,
    VkQueue graphics_queue,
    VkQueue present_queue,
    VkSemaphore render_complete_semaphore,
    u32 present_image_index) {
    
    // Return the image to the swapchain for presentation
    VkPresentInfoKHR present_info = { 0 };
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.pNext = 0;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &render_complete_semaphore;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &swapchain->handle;
    present_info.pImageIndices = &present_image_index;

    VkResult res = vkQueuePresentKHR(present_queue, &present_info);

    if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR)
        vulkan_swapchain_recreate(context, context->framebuffer_width, context->framebuffer_height, swapchain);
    else if (res != VK_SUCCESS)
        VFATAL("Failed to present swapchain image!");

    // increment (and loop) the index
    context->current_frame = (context->current_frame + 1) % swapchain->max_frames_in_flight;
}

void create(vulkan_context* context, u32 width, u32 height, vulkan_swapchain* out_swapchain) {
    VkExtent2D swapchain_extend = { width,height };
    out_swapchain->max_frames_in_flight = 2; // 2 frames rendered to 1 currently displayed

    b8 found = FALSE;
    for (u32 idx = 0; idx != context->device.swapchain_support.format_count; ++idx) {
        VkSurfaceFormatKHR format = context->device.swapchain_support.formats[idx];
        // Preffered formats
        if (format.format == VK_FORMAT_B8G8R8A8_UNORM
            && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            out_swapchain->format = format;
            found = TRUE;
            break;
        }
    }

    if (!found) {
        out_swapchain->format = context->device.swapchain_support.formats[0];
    }

    VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR; // this one is the default one
    for (u32 idx = 0; idx != context->device.swapchain_support.present_mode_count; ++idx) {
        VkPresentModeKHR mode = context->device.swapchain_support.present_mode[idx];
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR) { // triple buffering
            present_mode = mode;
            break;
        }
    }

    // Requery swapchain support capabilities to have them
    vulkan_device_query_swapchain_support(context->device.physical_device, context->surface, &context->device.swapchain_support);

    // Swapchain extent
    if (context->device.swapchain_support.capabilities.currentExtent.width != UINT32_MAX) {
        swapchain_extend = context->device.swapchain_support.capabilities.currentExtent;
    }
    
    // Clamp to a value allowed by the GPU
    VkExtent2D extend_min = context->device.swapchain_support.capabilities.minImageExtent;
    VkExtent2D extend_max = context->device.swapchain_support.capabilities.maxImageExtent;
    swapchain_extend.width = VCLAMP(swapchain_extend.width, extend_min.width, extend_max.width);
    swapchain_extend.height = VCLAMP(swapchain_extend.height, extend_min.height, extend_max.height);

    // Get image count
    u32 image_count = context->device.swapchain_support.capabilities.minImageCount + 1;
    if (context->device.swapchain_support.capabilities.maxImageCount > 0 &&
        image_count > context->device.swapchain_support.capabilities.maxImageCount) {
        image_count = context->device.swapchain_support.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR swapchain_create_info = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
    swapchain_create_info.pNext = 0;
    swapchain_create_info.surface = context->surface;
    swapchain_create_info.minImageCount = image_count;
    swapchain_create_info.imageFormat = out_swapchain->format.format;
    swapchain_create_info.imageColorSpace = out_swapchain->format.colorSpace;
    swapchain_create_info.imageExtent = swapchain_extend;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    // Set up queue family indices
    if (context->device.graphics_queue_index != context->device.present_queue_index) {
        u32 queue_indices[] = {
            (u32)context->device.graphics_queue_index,
            (u32)context->device.present_queue_index
        };
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchain_create_info.queueFamilyIndexCount = 2;
        swapchain_create_info.pQueueFamilyIndices = queue_indices;
    }
    else {
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchain_create_info.queueFamilyIndexCount = 0;
        swapchain_create_info.pQueueFamilyIndices = 0;
    }

    swapchain_create_info.preTransform = context->device.swapchain_support.capabilities.currentTransform;
    swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_create_info.presentMode = present_mode;
    swapchain_create_info.clipped = VK_TRUE;
    swapchain_create_info.oldSwapchain = 0;

    VkResult res = vkCreateSwapchainKHR(context->device.logical_device, &swapchain_create_info, context->allocator, &out_swapchain->handle);
    if (res != VK_SUCCESS) {
        VFATAL("Could not create swapchain");
    }

    // Create images + image views
    context->current_frame = 0;
    out_swapchain->image_count = 0;

    res = vkGetSwapchainImagesKHR(context->device.logical_device, out_swapchain->handle, &out_swapchain->image_count, 0);
    VK_CHECK(res);
    if (!out_swapchain->images) {
        out_swapchain->images = (VkImage*)vallocate(sizeof(VkImage) * out_swapchain->image_count, MEMORY_TAG_RENDERER);
    }
    if (!out_swapchain->views) {
        out_swapchain->views = (VkImageView*)vallocate(sizeof(VkImageView) * out_swapchain->image_count, MEMORY_TAG_RENDERER);
    }
    res = vkGetSwapchainImagesKHR(context->device.logical_device, out_swapchain->handle, &out_swapchain->image_count, out_swapchain->images);
    VK_CHECK(res);

    for (u32 idx = 0; idx != out_swapchain->image_count; ++idx) {
        VkImageViewCreateInfo image_view_info = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
        image_view_info.pNext = 0;
        image_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_info.format = out_swapchain->format.format;
        image_view_info.image = out_swapchain->images[idx];
        image_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        image_view_info.subresourceRange.baseMipLevel = 0;
        image_view_info.subresourceRange.levelCount = 1;
        image_view_info.subresourceRange.baseArrayLayer = 0;
        image_view_info.subresourceRange.layerCount = 1;

        res = vkCreateImageView(context->device.logical_device, &image_view_info, context->allocator, &out_swapchain->views[idx]);
        VK_CHECK(res);
    }

    // Depth buffer
    if (!vulkan_device_detect_depth_format(&context->device)) {
        context->device.depth_format = VK_FORMAT_UNDEFINED;
        VFATAL("Could not detect depth format");
    }

    vulkan_image_create(
        context,
        VK_IMAGE_TYPE_2D,
        width,
        height,
        context->device.depth_format,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        TRUE,
        VK_IMAGE_ASPECT_DEPTH_BIT,
        &out_swapchain->depth_attachment);
}

void destroy(vulkan_context* context, vulkan_swapchain* swapchain) {
    // Destroy depth image
    VINFO("Destroying depth attachment of swapchain...");
    vulkan_image_destroy(context, &swapchain->depth_attachment);
    VINFO("Destroyed depth attachment of swapchain");


    // Destroy image views
    VINFO("Destroying image views of swapchain...")
    for (u32 idx = 0; idx != swapchain->image_count; ++idx) {
        vkDestroyImageView(context->device.logical_device, swapchain->views[idx], context->allocator);
    }
    VINFO("Destroyed image views of swapchain");

    // Destroy swapchain
    vkDestroySwapchainKHR(context->device.logical_device, swapchain->handle, context->allocator);
}