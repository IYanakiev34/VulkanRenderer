#include "vulkan_device.h"
#include "core/logger.h"
#include "core/vstring.h"
#include "core/vmemory.h"
#include "containers/darray.h"
#include "core/vassert.h"


typedef struct vulkan_physical_device_requirments {
    b8 graphics;
    b8 present;
    b8 compute;
    b8 transfer;
    const char** device_extensions; // darray
    b8 sampler_anisotropy;
    b8 discrete_gpu;
} vulkan_physical_device_requirments;

typedef struct vulkan_physical_device_queue_family_info {
    u32 graphics_family_index;
    u32 present_family_index;
    u32 compute_family_index;
    u32 transfer_family_index;
} vulkan_physical_device_queue_family_info;

// Forward declaration of functions
b8 select_physical_device(vulkan_context* context);

b8 physical_device_meets_requirments(
    VkPhysicalDevice device,
    VkSurfaceKHR surface,
    const VkPhysicalDeviceProperties *properties,
    const VkPhysicalDeviceFeatures* features,
    const vulkan_physical_device_requirments* requirements,
    vulkan_physical_device_queue_family_info* out_queue_family_info,
    vulkan_swapchain_support_info* out_swapchain_info
);

b8 vulkan_device_create(vulkan_context* context) {
    if (!select_physical_device(context))
        return FALSE;

    VINFO("Creating logical device...");
    // NOTE: no additional queues for shared indices
    b8 present_shares_graphics_queue = context->device.present_queue_index == context->device.graphics_queue_index;
    b8 transfer_shares_graphics_queue = context->device.transfer_queue_index == context->device.graphics_queue_index;
    u32 index_count = 1;

    if (!present_shares_graphics_queue)
        ++index_count;

    if (!transfer_shares_graphics_queue)
        ++index_count;

    u32* indices = vallocate(sizeof(u32) * index_count, MEMORY_TAG_RENDERER);
    u8 index = 0;
    indices[index++] = context->device.graphics_queue_index;
    if (!present_shares_graphics_queue)
        indices[index++] = context->device.present_queue_index;

    if (!transfer_shares_graphics_queue)
        indices[index++] = context->device.transfer_queue_index;

    // TODO: add compute when needed
    VkDeviceQueueCreateInfo* queue_create_info = vallocate(sizeof(VkDeviceQueueCreateInfo) * index_count, MEMORY_TAG_RENDERER);

    f32 queue_priority = 1.0f;
    f32 queue_priority_arr[2] = { 1.0f,1.0f };
    for (u32 idx = 0; idx != index_count; ++idx) {
        queue_create_info[idx].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info[idx].queueFamilyIndex = indices[idx];
        queue_create_info[idx].queueCount = 1;
        /* Not all GPUs support that
        if (indices[idx] == context->device.graphics_queue_index)
            queue_create_info[idx].queueCount = 2;
        */
        queue_create_info[idx].flags = 0;
        queue_create_info[idx].pNext = 0;
        /*
        if (indices[idx] == context->device.graphics_queue_index)
            queue_create_info[idx].pQueuePriorities = queue_priority_arr;
        else*/
        queue_create_info[idx].pQueuePriorities = &queue_priority;
    }

    // Request device features
    // TODO: should be configuration driven
    VkPhysicalDeviceFeatures device_features = { 0 };   
    device_features.samplerAnisotropy = VK_TRUE;
 


    VkDeviceCreateInfo device_create_info = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
    device_create_info.queueCreateInfoCount = index_count;
    device_create_info.pQueueCreateInfos = queue_create_info;
    device_create_info.pEnabledFeatures = &device_features;
    device_create_info.enabledExtensionCount = 1;
    const char* extension_name = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
    device_create_info.ppEnabledExtensionNames = &extension_name;

    // Depreceated, do not use
    device_create_info.enabledLayerCount = 0;
    device_create_info.ppEnabledLayerNames = 0;

    VkResult res = vkCreateDevice(context->device.physical_device, &device_create_info, context->allocator, &context->device.logical_device);
    VK_CHECK(res);
    VINFO("Logical Device created!");

    // Get the queues and populate them in the context                             HARD CODED FOR NOW
    vkGetDeviceQueue(context->device.logical_device, context->device.graphics_queue_index, 0, &context->device.graphics_queue);
    vkGetDeviceQueue(context->device.logical_device, context->device.present_queue_index, 0, &context->device.present_queue);
    vkGetDeviceQueue(context->device.logical_device, context->device.transfer_queue_index, 0, &context->device.transfer_queue);
    // TODO: get the compute queue
    VINFO("Queues obtained!");

    // Create command pool for graphics
    VkCommandPoolCreateInfo pool_info = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
    pool_info.pNext = 0;
    pool_info.queueFamilyIndex = context->device.graphics_queue_index;
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    res = vkCreateCommandPool(context->device.logical_device, &pool_info, context->allocator, &context->device.graphics_command_pool);
    VK_CHECK(res);
    VINFO("Graphics command pool created");

    // Free queue and index memory
    vfree(queue_create_info, sizeof(VkDeviceQueueCreateInfo) * index_count, MEMORY_TAG_RENDERER);
    vfree(indices, sizeof(u32) * index_count, MEMORY_TAG_RENDERER);

    return TRUE;
}

void vulkan_device_destroy(vulkan_context* context) {
    context->device.graphics_queue = 0;
    context->device.transfer_queue = 0;
    context->device.present_queue = 0;
    // Same for compute queue

    VINFO("Destroying command pools...");
    vkDestroyCommandPool(context->device.logical_device, context->device.graphics_command_pool, context->allocator);

    VINFO("Destroying logical device...");
    if (context->device.logical_device) {
        vkDestroyDevice(context->device.logical_device, context->allocator);
    }


    VINFO("Releasing physical device resources...");

    if (context->device.swapchain_support.formats) {
        vfree(
            context->device.swapchain_support.formats,
            sizeof(VkSurfaceFormatKHR) * context->device.swapchain_support.format_count,
            MEMORY_TAG_RENDERER);
        context->device.swapchain_support.formats = 0;
        context->device.swapchain_support.format_count = 0;
    }

    if (context->device.swapchain_support.present_mode) {
        vfree(
            context->device.swapchain_support.present_mode,
            sizeof(VkPresentModeKHR) * context->device.swapchain_support.present_mode_count,
            MEMORY_TAG_RENDERER);
        context->device.swapchain_support.present_mode = 0;
        context->device.swapchain_support.present_mode_count = 0;
    }

    vzero_memory(&context->device.swapchain_support.capabilities,
                sizeof(VkSurfaceCapabilitiesKHR));
    context->device.compute_queue_index = 0;
    context->device.graphics_queue_index = 0;
    context->device.transfer_queue_index = 0;
    context->device.present_queue_index = 0;
}

void vulkan_device_query_swapchain_support(VkPhysicalDevice physical_device,
    VkSurfaceKHR surface,
    vulkan_swapchain_support_info* out_support_info) {

    // Surface capabilities
    VkResult res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &out_support_info->capabilities);
    VK_CHECK(res);

    // Surface formats
    {
        res = vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &out_support_info->format_count, out_support_info->formats);
        VK_CHECK(res);
        if (out_support_info->format_count != 0) {
            if (!out_support_info->formats)
            {
                out_support_info->formats = vallocate(sizeof(VkSurfaceFormatKHR) * out_support_info->format_count, MEMORY_TAG_RENDERER);
            }

            res = vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &out_support_info->format_count, out_support_info->formats);
            VK_CHECK(res);
        }
    }

    // Present modes
    {
        res = vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &out_support_info->present_mode_count, out_support_info->present_mode);
        VK_CHECK(res);
        if (out_support_info->present_mode_count != 0) {
            if (!out_support_info->present_mode) {
                out_support_info->present_mode = vallocate(sizeof(VkPresentModeKHR) * out_support_info->present_mode_count, MEMORY_TAG_RENDERER);
            }

            res = vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &out_support_info->present_mode_count, out_support_info->present_mode);
            VK_CHECK(res);
        }
    }
}

b8 vulkan_device_detect_depth_format(vulkan_device* device) {
    // Format candidates
    const u32 candidate_count = 3;
    VkFormat candidates[3] = {
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT
    };

    u32 flags = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
    for (u32 idx = 0; idx != candidate_count; ++idx) {
        VkFormatProperties format_properties;
        vkGetPhysicalDeviceFormatProperties(device->physical_device, candidates[idx], &format_properties);
        if ((format_properties.linearTilingFeatures & flags) == flags) {
            device->depth_format = candidates[idx];
            return TRUE;
        }
        else if ((format_properties.optimalTilingFeatures & flags) == flags) {
            device->depth_format = candidates[idx];
            return TRUE;
        }
    }

    return FALSE;
}

///////////////////////////////////////////////////
//          PHYSICAL DEVICE SELECTION           //
/////////////////////////////////////////////////

b8 select_physical_device(vulkan_context* context) {
    u32 device_count = 0;
    vkEnumeratePhysicalDevices(context->instance, &device_count, 0);
    if (device_count == 0) {
        VFATAL("No physical devices on the system which support Vulkan");
        return FALSE;
    }


    VkPhysicalDevice* devices = vallocate(sizeof(VkPhysicalDevice) * device_count, MEMORY_TAG_RENDERER);
    VkResult res = vkEnumeratePhysicalDevices(context->instance, &device_count, devices);
    VK_CHECK(res);

    // TODO: requirments driven by engine configuration
    vulkan_physical_device_requirments requirements;
    requirements.graphics = TRUE;
    requirements.present = TRUE;
    // TODO: enable this if compute required
    //requirements.compute = TRUE;
    requirements.transfer = TRUE;
    requirements.discrete_gpu = TRUE;
    requirements.device_extensions = darray_create(const char*);
    const char* swapchain_ext = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
    darray_push(requirements.device_extensions, swapchain_ext);
    requirements.sampler_anisotropy = TRUE;

    for (u32 idx = 0; idx != device_count; ++idx) {
        VkPhysicalDeviceProperties device_properties;
        vkGetPhysicalDeviceProperties(devices[idx], &device_properties);

        VkPhysicalDeviceFeatures device_features;
        vkGetPhysicalDeviceFeatures(devices[idx], &device_features);

        VkPhysicalDeviceMemoryProperties device_memory;
        vkGetPhysicalDeviceMemoryProperties(devices[idx], &device_memory);

        vulkan_physical_device_queue_family_info queue_infos;

        b8 meets_requirments = physical_device_meets_requirments(devices[idx], context->surface,
            &device_properties, &device_features, &requirements,&queue_infos, &context->device.swapchain_support);

        if (meets_requirments) {
            VINFO("Selected device: '%s'.", device_properties.deviceName);

            // GPU Type
            switch (device_properties.deviceType) {
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                VINFO("Device type is integrated GPU");
                break;
            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                VINFO("Device type is discrete GPU");
                break;
            case VK_PHYSICAL_DEVICE_TYPE_OTHER:
                VINFO("Device type is other");
                break;
            case VK_PHYSICAL_DEVICE_TYPE_CPU:
                VINFO("Device type is CPU");
                break;
            }

            // Driver info
            VINFO("GPU Driver version: %d.%d.%d",
                VK_VERSION_MAJOR(device_properties.driverVersion),
                VK_VERSION_MINOR(device_properties.driverVersion),
                VK_VERSION_PATCH(device_properties.driverVersion));

            // Vulkan API version
            VINFO("Vulkan API version: %d.%d.%d",
                VK_VERSION_MAJOR(device_properties.apiVersion),
                VK_VERSION_MINOR(device_properties.apiVersion),
                VK_VERSION_PATCH(device_properties.apiVersion));

            // Memory information
            for (u32 midx = 0; midx != device_memory.memoryHeapCount; ++midx) {
                f32 memory_size_gib = (((f32)device_memory.memoryHeaps[midx].size) / 1024.f / 1024.f / 1024.f);
                if (device_memory.memoryHeaps[midx].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
                    VINFO("Local GPU memory: %.2f GiB", memory_size_gib);
                }
                else {
                    VINFO("Shared system memory: %.2f", memory_size_gib);
                }
            }
            
            // Set device properties in context
            context->device.physical_device = devices[idx];
            // Keep copy of system properties
            context->device.properties = device_properties;
            context->device.features = device_features;
            context->device.memory = device_memory;

            // Queues
            context->device.graphics_queue_index = queue_infos.graphics_family_index;
            context->device.present_queue_index = queue_infos.present_family_index;
            context->device.transfer_queue_index = queue_infos.transfer_family_index;
            // TODO: set compute index later when needed
            //context->device.compute_queue_index = queue_infos.compute_family_index;
            break;
        }
    }

    // Ensure device was selected
    if (!context->device.physical_device) {
        VERROR("No physical devices were found which meet the requirments");
        return FALSE;
    }
    
    darray_destroy(requirements.device_extensions);

    vfree(devices, sizeof(VkPhysicalDevice)* device_count, MEMORY_TAG_RENDERER);

    return TRUE;
}

b8 physical_device_meets_requirments(
    VkPhysicalDevice device,
    VkSurfaceKHR surface,
    const VkPhysicalDeviceProperties* properties,
    const VkPhysicalDeviceFeatures* features,
    const vulkan_physical_device_requirments* requirements,
    vulkan_physical_device_queue_family_info* out_queue_family_info,
    vulkan_swapchain_support_info* out_swapchain_info) {

    // Absurd number to signify basically NONE
    out_queue_family_info->graphics_family_index = 30000;
    out_queue_family_info->present_family_index = 30000;
    out_queue_family_info->compute_family_index = 30000;
    out_queue_family_info->transfer_family_index = 30000;

    if (requirements->discrete_gpu) {
        if (properties->deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            VINFO("Device is not a discrete GPU, and one is required. Skipping");
            return FALSE;
       }
    }

    u32 queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, 0);
    VkQueueFamilyProperties* queue_properties = vallocate(sizeof(VkQueueFamilyProperties) * queue_family_count, MEMORY_TAG_RENDERER);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_properties);

    // Look at each queue and see what it supports
    VINFO("| Graphics | Present   | Compute   | Transfer  | Name    |");
    u8 min_transfer_score = 255;
    for (u32 idx = 0; idx != queue_family_count; ++idx) {
        u8 current_transfer_score = 0;
        
        // Graphics queue?
        if (queue_properties[idx].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            out_queue_family_info->graphics_family_index = idx;
            ++current_transfer_score;
        }

        // Compute queue?
        if (queue_properties[idx].queueFlags & VK_QUEUE_COMPUTE_BIT) {
            out_queue_family_info->compute_family_index = idx;
            ++current_transfer_score;
        }

        // Transfer queue?
        if (queue_properties[idx].queueFlags & VK_QUEUE_TRANSFER_BIT) {

            if (current_transfer_score <= min_transfer_score) {
                min_transfer_score = current_transfer_score;
                out_queue_family_info->transfer_family_index = idx;
            }
        }

        // Present queue?
        VkBool32 supports_present = VK_FALSE;
        VkResult res = vkGetPhysicalDeviceSurfaceSupportKHR(device, idx, surface, &supports_present);
        VK_CHECK(res);
        if (supports_present) {
            out_queue_family_info->present_family_index = idx;
        }
    }

    // Print info on device
    VINFO("     %d |        %d |        %d |        %d |     %s |",
        out_queue_family_info->graphics_family_index,
        out_queue_family_info->present_family_index,
        out_queue_family_info->compute_family_index,
        out_queue_family_info->transfer_family_index,
        properties->deviceName);

    if (
        (!requirements->graphics || (requirements->graphics && out_queue_family_info->graphics_family_index != 30000)) &&
        (!requirements->compute || (requirements->compute && out_queue_family_info->compute_family_index != 30000)) &&
        (!requirements->transfer || (requirements->transfer && out_queue_family_info->transfer_family_index != 30000)) &&
        (!requirements->present || (requirements->present && out_queue_family_info->present_family_index != 30000))
        )
    {
        VINFO("Device meets queue requirments");
        VTRACE("Graphics Queue Family Index:    %i", out_queue_family_info->graphics_family_index);
        VTRACE("Compute Queue Family Index:     %i", out_queue_family_info->compute_family_index);
        VTRACE("Present Queue Family Index:     %i", out_queue_family_info->present_family_index);
        VTRACE("Transfer Queue Family Index:    %i", out_queue_family_info->transfer_family_index);

        // Swapchain support information
        vulkan_device_query_swapchain_support(device, surface, out_swapchain_info);

        if (out_swapchain_info->format_count < 1 || out_swapchain_info->present_mode_count < 1) {
            if (out_swapchain_info->formats) {
                vfree(out_swapchain_info->formats, sizeof(VkSurfaceFormatKHR) * out_swapchain_info->format_count, MEMORY_TAG_RENDERER); // free formats
            }

            if (out_swapchain_info->present_mode) {
                vfree(out_swapchain_info->present_mode, sizeof(VkPresentModeKHR) * out_swapchain_info->present_mode_count, MEMORY_TAG_RENDERER); // free present modes
            }

            VINFO("Required swap chain support missing. Skipping device.");
            return FALSE;
        }

        // Device extensions
        if (requirements->device_extensions) {
            u32 available_extensions_count = 0;
            VkExtensionProperties* available_extensions = 0;

            VkResult res = vkEnumerateDeviceExtensionProperties(device, 0, &available_extensions_count, 0);
            VK_CHECK(res);
            if (available_extensions_count > 0) {
                available_extensions = vallocate(sizeof(VkExtensionProperties) * available_extensions_count, MEMORY_TAG_RENDERER);


                res = vkEnumerateDeviceExtensionProperties(device, 0, &available_extensions_count, available_extensions);
                VK_CHECK(res);

                u32 required_extensions_count = (u32)darray_length(requirements->device_extensions);
                for (u32 idx = 0; idx != required_extensions_count; ++idx) {
                    b8 found = FALSE;
                    for (u32 j = 0; j != available_extensions_count; ++j) {
                        if (strings_equal(requirements->device_extensions[idx], available_extensions[j].extensionName)) {
                            found = TRUE;
                            break;
                        }
                    }

                    if (!found) {
                        VINFO("Required extension not found: '%s'. Skipping device", requirements->device_extensions[idx]);
                        return FALSE;
                    }
                }
            }
            vfree(available_extensions, sizeof(VkExtensionProperties) * available_extensions_count, MEMORY_TAG_RENDERER); // free extensions
        }

        // Sampler anisotropy
        if (requirements->sampler_anisotropy && !features->samplerAnisotropy) {
            VINFO("Device does not support sampler anisotropy. Skipping device");
            return FALSE;
        }

        return TRUE;
    }

    vfree(queue_properties, sizeof(VkQueueFamilyProperties)* queue_family_count, MEMORY_TAG_RENDERER); // free queues
    return FALSE;
}
