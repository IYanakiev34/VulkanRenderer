#pragma once

#include "defines.h"

// Keep track of platform specific handles, etc
typedef struct platform_state {
    void* internal_state;
} platform_state;

/**
* Responsible for creation of a window, if the platform has a concept of a window.
* If not we should just create the application state.
* @param plat_state - The platform state, basically different handles like windows
* @param application_name - The name displayed on the window if the platform has a concept of window
* @param x - The X origin on the screen of the window
* @param y - The Y origin on the screen of the window
* @param width - The width of the window
* @param height - The height of the window
* 
* @return b8 - True of false depending on the state of initialization
*/
b8 platform_startup(
    platform_state* plat_state,
    const char* application_name,
    i32 x,
    i32 y,
    i32 width,
    i32 height);

/*
* Responsible for shutting down the application. Remove all loggers, clear all queues. Close windows. Cleanup any state.
* 
* @param plat_state - The current state of the platform and all of it's handles
*/
void    platform_shutdown(platform_state* plat_state);

/*
* Resonsiple for TODO:
*/
b8      platform_pump_message(platform_state* plat_state);

/*
* Performs platform specific memory allocation. In the future with custom allocators per platform.
* 
* @param size - The size of the block we need
* @param aligned - Indicates if we want the memory to be aligned
* 
* @return void* - Pointer to the allocated block of memory
*/
void*   platform_allocate(u64 size, b8 aligned);

/*
* Performs platform specific free of a memory block.
* 
* @param block - A void pointer to the block of memory we wish to free
* @param size - The size of the block of memory (in bytes)
*/
void    platform_free(void* block, b8 aligned);

/*
* Perform a memset of 0 on a block of memory in a platform agnostic way.
* 
* @param block - A void pointer to the block of memory we wish to set to 0
* @param size - The size of the block of memory (in bytes)
* 
* @return void* - A pointer to the block of memory we have set to 0
*/
void*   platform_zero_memory(void* block, u64 size);

/*
* Performs a platform specific copying of memory from one block to another.
* 
* @param dest - The destination of the copied memory.
* @param src - The source from where we will copy the memory.
* @param size - The amount of memory in bytes that will be copied
* 
* @return void * - A pointer to the destination block of memory
*/
void*   platform_copy_memory(void* dest, void* src, u64 size);

/*
* Performs a platform specific setting of memory.
* 
* @param block - A void pointer to the block of memory that we with to set
* @param value - The value that we with to place within the block of memory
* @param size - The size in bytes that we will set
* 
* @return void* - A pointer to the block of memoery we have set to <value>
*/
void*   platform_set_memory(void* block, i32 value, u64 size);

/*
* Platform specific console write. If the platform has a console to write on.
* 
* @param msg - The message that we want to output to the console.
* @param color - The color that we will display the message in. Color is closely tied to the log level
*/
void platform_console_write(const char* msg, u8 color);

/*
* Platform specific console write of an error. If the platform has a console to write on.
* 
* @param msg - The message that we want to output to the console as an error.
* @param color - The color of the displayed message (will be red)
*/
void platform_console_write_error(const char* msg, u8 color);

/*
* Platform specific implementation of time. We will obtain the absolute time of the system.
* @return f64 - Time time in milliseconds
*/
f64 platform_get_absolute_time();

/*
* Platform specific implementation of thread sleep. The thread that calls this method will
* sleep for the amount of milliseconds passed into the function.
* 
* @param ms - The milliseconds you want the thread to sleep
*/
void platform_sleep(u64 ms);