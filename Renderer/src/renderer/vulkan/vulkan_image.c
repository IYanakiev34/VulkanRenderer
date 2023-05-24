#include "vulkan_image.h"
#include "vulkan_device.h"
#include "core/logger.h"
#include "core/vmemory.h"

void vulkan_image_create(
    vulkan_context* context,
    VkImageType image_type,
    u32 width,
    u32 height,
    VkFormat format,
    VkImageTiling tiling,
    VkImageUsageFlags usage,
    VkMemoryPropertyFlags memory_flags,
    b32 create_view,
    VkImageAspectFlags view_aspect_flags,
    vulkan_image* out_image) {
    out_image->width = width;
    out_image->height = height;

    VkImageCreateInfo image_info = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    image_info.pNext = 0;
    image_info.imageType = VK_IMAGE_TYPE_2D; // TODO: could be different type
    image_info.format = format;
    image_info.extent.width = width;
    image_info.extent.height = height;
    image_info.extent.depth = 1; // TODO: support configurable depth
    image_info.mipLevels = 4; // TODO: suport mipmapping
    image_info.arrayLayers = 1;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT; // TODO: configurable sample count
    image_info.tiling = tiling;
    image_info.usage = usage;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // TODO: configurable sharing mode
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkResult res = vkCreateImage(context->device.logical_device, &image_info, context->allocator, &out_image->handle);
    VK_CHECK(res);
    VkMemoryRequirements memory_requirments;
    vkGetImageMemoryRequirements(context->device.logical_device, out_image->handle, &memory_requirments);

    i32 memory_type = context->find_memory_index(memory_requirments.memoryTypeBits, memory_flags);
    if (memory_type == -1) {
        VERROR("Required memory type not found! Image not valid");
    }

    // Allocate memory
    VkMemoryAllocateInfo allocate_info = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
    allocate_info.memoryTypeIndex = memory_type;
    allocate_info.pNext = 0;
    allocate_info.allocationSize = memory_requirments.size;

    res = vkAllocateMemory(context->device.logical_device, &allocate_info, context->allocator, &out_image->memory);
    VK_CHECK(res);

    // Bind memory
    res = vkBindImageMemory(context->device.logical_device, out_image->handle, out_image->memory, 0); // TODO: configurable offset
    VK_CHECK(res);

    if (create_view) {
        out_image->view = 0;
        vulkan_image_view_create(context, format, out_image, view_aspect_flags);
    }
}

void vulkan_image_view_create(
    vulkan_context* context,
    VkFormat format,
    vulkan_image* image,
    VkImageAspectFlags aspect_flags) {
    VkImageViewCreateInfo view_info = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    view_info.pNext = 0;
    view_info.format = format;
    view_info.image = image->handle;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.subresourceRange.aspectMask = aspect_flags;

    // TODO: configurable
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.layerCount = 1;
    view_info.subresourceRange.levelCount = 0;
    view_info.subresourceRange.levelCount = 1;

    VkResult res = vkCreateImageView(context->device.logical_device, &view_info, context->allocator, &image->view);
    VK_CHECK(res);
}

void vulkan_image_destroy(vulkan_context* context, vulkan_image* image) {
    if (image->view) {
        vkDestroyImageView(context->device.logical_device, image->view, context->allocator);
        image->view = 0;
    }
    if (image->memory) {
        vkFreeMemory(context->device.logical_device, image->memory, context->allocator);
        image->memory = 0;
    }
    if (image->handle) {
        vkDestroyImage(context->device.logical_device, image->handle, context->allocator);
        image->handle = 0;
    }
}