#pragma once
#include "vulkan_types.inl"

/*
* Creates the swapchain.
* 
* @param context - The vulkan context
* @param width - The width of the screen
* @param height - The height of the screen
* @param out_swapchain - The swapchain that will be created
*/
void vulkan_swapchain_create(
    vulkan_context * context,
    u32 width,
    u32 height,
    vulkan_swapchain* out_swapchain);

/* Recreates the swapchain. Usually called after a resize.
* @param context - The vulkan context
* @param width - The screen width
* @param height - The screen height
* @param swapchain - The swapchain to recreate
*/
void vulkan_swapchain_recreate(
    vulkan_context *context,
    u32 width,
    u32 height,
    vulkan_swapchain* swapchain);

/*
* Destroys a vulkan swapchain.
* 
* @param context - The vulkan context
* @param swapchain - The swapchain to destroy
*/
void vulkan_swapchain_destroy(
    vulkan_context* context,
    vulkan_swapchain* swapchain);

/*
* Gets the next image index from the swapchain.
* 
* @param context - The vulkan context
* @param swapchain - The swapchain to take an image index from
* @param timeout_ms - The time to wait if the image cannot be taken
* @param image_available_semaphore - The vulkan semaphore to check for image availability
* @param fence - The vulkan fence to synchronize application with gpu
* @param out_image_index - The image index that will be obtained
* 
* @return b8 - TRUE if image index was obatined, FALSE if error occured
*/
b8 vulkan_swapchain_acquire_next_image_index(
    vulkan_context* context,
    vulkan_swapchain* swapchain,
    u64 timeout_ms,
    VkSemaphore image_available_semaphore,
    VkFence fence,
    u32* out_image_index);

/*
* Present an image from the swapchain.
* 
* @param context - The vulkan context
* @param swapchain - The swapchain that will be presented from
* @param graphics_queue - The graphics queue handle
* @param present_queue - The present queue handle
* @param render_complete_semaphore - Indicates if rendering was finished
* @pram present_image_index - The image index to present
*/
void vulkan_swapchain_present(
    vulkan_context *context,
    vulkan_swapchain* swapchain, // TODO: this is in context probably do not need it
    VkQueue graphics_queue,
    VkQueue present_queue,
    VkSemaphore render_complete_semaphore,
    u32 present_image_index);