#pragma once
#include "vulkan_types.inl"


/*
* Creates vulkan image based on a specification. Could opt-in
* to also create an image view with the image itself. You need
* to pass a valid context otherwise the logical device will throw
* an error.
* 
* @param context - The vulkan context
* @param image_type - The vulkan type of the image (1D,2D,3D etc)
* @param width - The width of the image
* @param height - The height of the image
* @param format - The format of the image (Vulkan Spec Format)
* @param tiling - The memory tiling
* @param usage - How should the image be used
* @param memory_flags - Memory type of the image
* @param create_view - If you want to create a view with the image
* @param view_aspect_flags - Aspect flags of the image
* @param out_image - pointer to the out image
*/
void vulkan_image_create(
    vulkan_context *context,
    VkImageType image_type,
    u32 width,
    u32 height,
    VkFormat format,
    VkImageTiling tiling,
    VkImageUsageFlags usage,
    VkMemoryPropertyFlags memory_flags,
    b32 create_view,
    VkImageAspectFlags view_aspect_flags,
    vulkan_image* out_image);

/*
* Creates an image for for an image
* 
* @param context - Valid Vulkan context
* @param format - The format of the image view
* @param image - The image that the view will be created for
* @param aspect_flags - The aspect flags of the image
*/
void vulkan_image_view_create(
    vulkan_context* context,
    VkFormat format,
    vulkan_image* image,
    VkImageAspectFlags aspect_flags);

/*
* Destroys a vulkan image, along with potentially its view and memory
* 
* @param context - Valid vulkan context
* @param image - Valid vulkan image
*/
void vulkan_image_destroy(vulkan_context* context, vulkan_image* image);