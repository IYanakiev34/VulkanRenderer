#include "vmemory.h"
#include "platform/platform.h"
#include "logger.h"

#include <string.h>
#include <stdio.h>

struct memory_stats {
    u64 total_allocated;
    u64 tagged_allocations[MEMORY_TAG_MAXTAGS];
};

static struct memory_stats stats;

static const char* memory_tag_strings[MEMORY_TAG_MAXTAGS] = {
    "UKNOWN     ",
    "ARRAY      ",
    "DARRAY     ",
    "DICT       ",
    "RING_QUEUE ",
    "STRING     ",
    "BST        ",
    "APPLICATION",
    "JOB        ",
    "TEXTURE    ",
    "MAT_INST   ",
    "RENDERER   ",
    "GAME       ",
    "TRANSFORM  ",
    "ENTITY     ",
    "SCENE      ",
    "ENTITY_NODE",
};

void initialize_memory() {
    platform_zero_memory(&stats, sizeof(stats));
}

void shutdown_memory() {
}

void* vallocate(u64 size, memory_tag tag) {
    if (tag == MEMORY_TAG_UNKNOWN) {
        VWARN("vallocate called using MEMORY_TAG_UNKNOWN. Re-class this allocation");
    }

    // Update memory stats
    stats.total_allocated += size;
    stats.tagged_allocations[tag] += size;

    // TODO: Handle aligned memory
    void* block = platform_allocate(size, FALSE);
    platform_zero_memory(&block, size);
    return block;
}

void vfree(void* block, u64 size, memory_tag tag) {
    if (tag == MEMORY_TAG_UNKNOWN){
        VWARN("vfree called using MEMORY_TAG_UNKNOWN. Re-class this free");
    }

    stats.total_allocated -= size;
    stats.tagged_allocations[tag] -= size;

    //TODO: Handle aligned memory
    platform_free(block, FALSE);
}

void* vzero_memory(void* block, u64 size) {
    return platform_zero_memory(block, size);
}

void* vcopy_memory(void* dest, void* src, u64 size) {
    return platform_copy_memory(dest, src, size);
}

void* vset_memory(void* block, i32 value, u64 size) {
    return platform_set_memory(block, value, size);
}

char* get_memory_usage_str() {
    const u64 gib = 1024 * 1024 * 1024;
    const u64 mib = 1024 * 1024;
    const u64 kib = 1024;

    char buffer[5000] = "System memory use (tagged):\n";
    u64 offset = strlen(buffer);

    for (u32 idx = 0; idx != MEMORY_TAG_MAXTAGS; ++idx) {
        char unit[4] = "XiB";
        float amount = 1.f;

        if (stats.tagged_allocations[idx] >= gib) {
            unit[0] = 'G';
            amount = stats.tagged_allocations[idx] / (float)gib;
        }
        else if (stats.tagged_allocations[idx] >= mib) {
            unit[0] = 'M';
            amount = stats.tagged_allocations[idx] / (float)mib;
        }
        else if (stats.tagged_allocations[idx] >= kib) {
            unit[0] = 'K';
            amount = stats.tagged_allocations[idx] / (float)kib;
        }
        else {
            unit[0] = 'B';
            unit[1] = 0;
            amount = (float)stats.tagged_allocations[idx];
        }

        i32 written = snprintf(buffer + offset, 5000 - offset, " %s: %.2f%s\n", memory_tag_strings[idx], amount, unit);
        offset += written;
    }

    char* out_string = _strdup(buffer);
    return out_string;
}

