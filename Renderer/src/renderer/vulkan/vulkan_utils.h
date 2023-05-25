#pragma once
#include "vulkan_types.inl"

/*
* Indicates if the passed result if a success or an error as defined in the Vulkan Spec.
* @returns TRUE if success;FALSE otherwise. Defaults to true for unknow types
*/
b8 vulkan_result_is_success(VkResult res);

/*
* Return the sring representation of a VkResult
* @param result - The result to get a string for
* @param get_extended - Indicates weather or not to return extended result
* @return The error code and/or extended errro message in string form. Defaults to success for unknow types
*/
const char* vulkan_result_string(VkResult res, b8 get_extended);