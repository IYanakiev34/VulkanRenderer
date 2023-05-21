#pragma once

/**
* Appends the names of the required extensions for the specific platform
* you are on now, to the names_darray.
* 
* @param names_darray - Dynamic array of const char * that will store the names
* of the required extensions for the specific platform
*/
void platform_get_required_extensions_names(char*** names_darray);