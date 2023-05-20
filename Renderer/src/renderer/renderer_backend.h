#pragma once
#include "renderer_types.inl"

struct platform_state;

/*
* Creates the proper renderer backend.
* 
* @param backend_type - The renderer api that we wish to use
* @param plat_state - The platform state of the applciation
* @param out_backend - Pointer to a renderer_backend which will be filled after the function has finished
* 
* @return b8 - TRUE is successfulyl created a backend renderer of some type, FALSE otherwise
*/
b8 renderer_backend_create(renderer_backend_type backend_type, struct platform_state* plat_state, renderer_backend* out_backend);

/**
* Destroys a renderer backend. Removes any function pointers and eallocates the memory
*/
void renderer_backend_destroy(renderer_backend* backend);