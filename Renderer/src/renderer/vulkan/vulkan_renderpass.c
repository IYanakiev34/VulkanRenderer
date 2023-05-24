#include "vulkan_renderpass.h"
#include "core/logger.h"
#include "core/vmemory.h"

void vulkan_renderpass_create(
    vulkan_context* context,
    vulkan_renderpass* out_renderpass,
    f32 x, f32 y, u32 w, u32 h,
    f32 r, f32 g, f32 b, f32 a,
    f32 depth,
    u32 stencil) {

    // Subpass
    VkSubpassDescription main_pass = { 0 };
    main_pass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    u32 attachment_count = 2; // TODO: configurable
    VkAttachmentDescription* attachments = vallocate(sizeof(VkAttachmentDescription) * attachment_count, MEMORY_TAG_RENDERER);
    
    // Color attachment
    attachments[0].format = context->swapchain.format.format; // TODO: configurable
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT; // TODO: configurable
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    attachments[0].flags = 0;

    // Reference in the attachments array
    VkAttachmentReference color_attachment_reference;
    color_attachment_reference.attachment = 0;
    color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Subpass color
    main_pass.colorAttachmentCount = 1;
    main_pass.pColorAttachments = &color_attachment_reference;

    // Depth attachment. If there is one
    attachments[1].format = context->device.depth_format;
    attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    
    VkAttachmentReference deptch_stencil_reference;
    deptch_stencil_reference.attachment = 1;
    deptch_stencil_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // TODO: other attachment stype (input,resolve,preserve)

    // Subpass depth
    main_pass.pDepthStencilAttachment = &deptch_stencil_reference;

    // Input from shader
    main_pass.inputAttachmentCount = 0;
    main_pass.pInputAttachments = 0;

    // Attachments for multisampling color
    main_pass.pResolveAttachments = 0;

    // Attachments for next subpasses preserved
    main_pass.preserveAttachmentCount = 0;
    main_pass.pPreserveAttachments = 0;

    // Renderpass dependency TODO: configuration driven
    VkSubpassDependency dependency;
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependency.dependencyFlags = 0;

   
    VkRenderPassCreateInfo renderpass_info = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
    renderpass_info.pNext = 0;
    renderpass_info.attachmentCount = attachment_count; // TODO: configurable probably should be more
    renderpass_info.pAttachments = attachments;
    renderpass_info.subpassCount = 1;
    renderpass_info.pSubpasses = &main_pass;
    renderpass_info.dependencyCount = 1;
    renderpass_info.pDependencies = &dependency;
    renderpass_info.flags = 0;

    VkResult res = vkCreateRenderPass(context->device.logical_device, &renderpass_info, context->allocator, &out_renderpass->handle);
    vfree(attachments, sizeof(VkAttachmentDescription) * attachment_count, MEMORY_TAG_RENDERER);
    VK_CHECK(res);

}

void vulkan_renderpass_destroy(
    vulkan_context* context,
    vulkan_renderpass* renderpass) {
    if (renderpass && renderpass->handle) {
        vkDestroyRenderPass(context->device.logical_device, renderpass->handle, context->allocator);
        renderpass->handle = 0;
    }
}

void vulkan_renderpass_begin(
    vulkan_command_buffer* command_buffer,
    vulkan_renderpass* renderpass,
    VkFramebuffer frame_buffer) {

    VkRenderPassBeginInfo begin_info = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
    begin_info.pNext = 0;
    begin_info.renderPass = renderpass->handle;
    begin_info.framebuffer = frame_buffer;

    VkRect2D area;
    area.offset.x = (i32)renderpass->x;
    area.offset.y = (i32)renderpass->y;
    area.extent.width = (i32)renderpass->w;
    area.extent.height = (i32)renderpass->h;

    begin_info.renderArea = area;
    begin_info.clearValueCount = 1;
    
    VkClearValue clear_values[2];
    vzero_memory(clear_values, sizeof(VkClearValue) * 2);
    clear_values[0].color.float32[0] = renderpass->r;
    clear_values[0].color.float32[1] = renderpass->g;
    clear_values[0].color.float32[2] = renderpass->b;
    clear_values[0].color.float32[3] = renderpass->a;

    clear_values[1].depthStencil.depth = renderpass->depth;
    clear_values[1].depthStencil.stencil = renderpass->stencil;

    begin_info.clearValueCount = 2;
    begin_info.pClearValues = clear_values;

    vkCmdBeginRenderPass(command_buffer->handle, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
    command_buffer->state = COMMAND_BUFFER_STATE_IN_RENDER_PASS;

}

void vulkan_renderpas_end(
    vulkan_command_buffer* command_buffer,
    vulkan_renderpass* renderpass) {
    vkCmdEndRenderPass(command_buffer->handle);
    command_buffer->state = COMMAND_BUFFER_STATE_RECORDING;
}