#pragma once

#include "defines.h"

typedef enum memory_tag {
    MEMORY_TAG_UNKNOWN = 0,
    MEMORY_TAG_ARRAY,
    MEMORY_TAG_DARRAY,
    MEMORY_TAG_DICT,
    MEMORY_TAG_RING_QUEUE,
    MEMORY_TAG_STRING,
    MEMORY_TAG_BST,
    MEMORY_TAG_APPLICATION,
    MEMORY_TAG_JOB,
    MEMORY_TAG_TEXTURE,
    MEMORY_TAG_MATERIAL_INSTANCE,
    MEMORY_TAG_RENDERER,
    MEMORY_TAG_GAME,
    MEMORY_TAG_TRANSFORM,
    MEMORY_TAG_ENTITY,
    MEMORY_TAG_SCENE,
    MEMORY_TAG_ENTITY_NODE,

    MEMORY_TAG_MAXTAGS
} memory_tag;

/**
* Responsible for initializing the memory sub-system.
* TODO: Probably we will preallocate large block for the system.
* */
VAPI void initialize_memory();

/**
* Responsible for shutting down the memory system.
* TODO: probably it will clean up all used memory.
*/
VAPI void shutdown_memory();

/**
* Responsible for allocating memory.
* @param size - The size of the memory block to allocate in bytes
* @param tag - The type of memory that the system will allocate
*/
VAPI void* vallocate(u64 size, memory_tag tag);

/**
* Responsible for freeing a memory block.
* @param block - The block of memory that will be freed
* @param size - The size of the block of memory that will be freed (in bytes)
* @param tag - The type of the memory block
*/
VAPI void vfree(void* block, u64 size, memory_tag tag);

/**
* Responsible for zeroing out a memory block.
* @param block - The block of memory that will be set to 0
* @param size - The size of the memory block (in bytes)
*/
VAPI void* vzero_memory(void* block, u64 size);

/**
* Responsible for copying memory from one block to another given a size.
* @param dest - The destination memory block (the one where we will copy memory to)
* @param src - The source memory block (the one we will copy memory from)
* @param size - The amount of memory that will be copied (in bytes)
*/
VAPI void* vcopy_memory(void* dest, void* src, u64 size);

/**
* Responsible for setting a block of memory to some value.
* @param block - The block of memory that will be set
* @param value - The value that we will set the block of memory to
* @param size - The size of the block of memory (in bytes)
*/
VAPI void* vset_memory(void* block, i32 value, u64 size);

VAPI char* get_memory_usage_str();