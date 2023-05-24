#include "vulkan_command_buffer.h"
#include "core/vmemory.h"

void vulkan_command_buffer_allocate(
    vulkan_context* context,
    VkCommandPool pool,
    b8 is_primary,
    vulkan_command_buffer* out_command_buffer) {
    vzero_memory(out_command_buffer, sizeof(vulkan_command_buffer));

    VkCommandBufferAllocateInfo buffer_info = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    buffer_info.pNext = 0;
    buffer_info.commandPool = pool;
    buffer_info.level = is_primary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    buffer_info.commandBufferCount = 1;

    VkResult res = vkAllocateCommandBuffers(context->device.logical_device, &buffer_info, &out_command_buffer->handle);
    VK_CHECK(res);
    out_command_buffer->state = COMMAND_BUFFER_STATE_READY;
}

void vulkan_command_buffer_free(
    vulkan_context* context,
    VkCommandPool pool,
    vulkan_command_buffer* command_buffer) {
    vkFreeCommandBuffers(context->device.logical_device, pool, 1, &command_buffer->handle);
    command_buffer->state = COMMAND_BUFFER_STATE_NOT_ALLOCATED;
    command_buffer->handle = 0;
}

void vulkan_command_buffer_begin_recording(
    vulkan_command_buffer* command_buffer,
    b8 is_single_use,
    b8 is_renderpass_continue,
    b8 is_simultanious_use) {
    VkCommandBufferBeginInfo begin_info = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    begin_info.pNext = 0;
    
    if (is_single_use)
        begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    if (is_renderpass_continue)
        begin_info.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    if (is_simultanious_use)
        begin_info.flags |= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

    VkResult res = vkBeginCommandBuffer(command_buffer->handle, &begin_info);
    VK_CHECK(res);
    command_buffer->state = COMMAND_BUFFER_STATE_RECORDING;
}

void vulkan_command_buffer_end_recording(vulkan_command_buffer* command_buffer) {
    vkEndCommandBuffer(command_buffer->handle);
    command_buffer->state = COMMAND_BUFFER_STATE_RECORDING_ENDED;
}

void vulkan_command_buffer_update_submit(vulkan_command_buffer* command_buffer) {
    command_buffer->state = COMMAND_BUFFER_STATE_SUBMITTED;
}

void vulkan_command_buffer_reset(vulkan_command_buffer* command_buffer) {
    command_buffer->state = COMMAND_BUFFER_STATE_READY;
}

void vulkan_command_buffer_allocate_begin_single_use(
    vulkan_context* context,
    VkCommandPool pool,
    vulkan_command_buffer* out_command_buffer) {
    vulkan_command_buffer_allocate(context, pool, TRUE, out_command_buffer);
    vulkan_command_buffer_begin_recording(out_command_buffer, TRUE, FALSE, FALSE);

}

void vulkan_command_buffer_end_single_use(
    vulkan_context* context,
    VkCommandPool pool,
    vulkan_command_buffer* command_buffer,
    VkQueue queue) {
    vulkan_command_buffer_end_recording(command_buffer);

    // Submit to queue
    VkSubmitInfo submit_info = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
    submit_info.pNext = 0;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer->handle;

    VkResult res = vkQueueSubmit(queue, 1, &submit_info, 0);
    VK_CHECK(res);

    // Wait to finish
    res = vkQueueWaitIdle(queue);
    VK_CHECK(res);

    // Free command buffer
    vulkan_command_buffer_free(context, pool, command_buffer);
}