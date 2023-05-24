#include "vulkan_framebuffer.h"
#include "core/vmemory.h"

void vulkan_framebuffer_create(
    vulkan_context* context,
    vulkan_renderpass* renderpass,
    u32 width,
    u32 height,
    u32 attachment_count,
    VkImageView* attachments,
    vulkan_framebuffer* out_framebuffer) {

    out_framebuffer->attachments = vallocate(sizeof(VkImageView) * attachment_count, MEMORY_TAG_RENDERER);
    // Take a copy of the attachments
    for (u32 idx = 0; idx != attachment_count; ++idx) {
        out_framebuffer->attachments[idx] = attachments[idx];
    }

    out_framebuffer->renderpass = renderpass;
    out_framebuffer->attachment_count = attachment_count;

    VkFramebufferCreateInfo buffer_info = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
    buffer_info.pNext = 0;
    buffer_info.renderPass = out_framebuffer->renderpass->handle;
    buffer_info.attachmentCount = out_framebuffer->attachment_count;
    buffer_info.pAttachments = out_framebuffer->attachments;
    buffer_info.width = width;
    buffer_info.height = height;
    buffer_info.layers = 1;

    VkResult res = vkCreateFramebuffer(context->device.logical_device, &buffer_info, context->allocator, &out_framebuffer->handle);
    VK_CHECK(res);
}

void vulkan_framebuffer_destroy(
    vulkan_context* context,
    vulkan_framebuffer* framebuffer) {

    if (framebuffer->attachments) {
        vfree(framebuffer->attachments,sizeof(VkImageView) * framebuffer->attachment_count, MEMORY_TAG_RENDERER);
        framebuffer->attachments = 0;
    }

    vkDestroyFramebuffer(context->device.logical_device, framebuffer->handle, context->allocator);
    framebuffer->renderpass = 0;
    framebuffer->handle = 0;
    framebuffer->attachment_count = 0;
}