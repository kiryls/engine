#include "kmemory.h"

#include "core/logger.h"
#include "platform/platform.h"

// #todo Custom string lib
#include <string.h>
#include <stdio.h>

struct memory_stats {
    u64 total_allocated;
    u64 tagged_allocations[MEMORY_TAG_MAX_TAGS];
} memory_stats;

static const char* memory_tag_strings[MEMORY_TAG_MAX_TAGS] = {
    "UNKNOWN    ",
    "ARRAY      ",
    "DARRAY     ",
    "DICT       ",
    "RING_QUEUE ",
    "BST        ",
    "STRING     ",
    "APPLICATION",
    "JOB        ",
    "TEXTURE    ",
    "MAT_INST   ",
    "RENDERER   ",
    "GAME       ",
    "TRANSFORM  ",
    "ENTITY     ",
    "ENTITY_NODE",
    "SCENE      "};

static struct memory_stats stats;

void initialize_memory() {
    platform_zero_memory(&stats, sizeof(stats));
}

void shutdown_memory() {
}

void* kallocate(u64 size, memory_tag tag) {
    if (tag == MEMORY_TAG_UNKNOWN) {
        KWARN("kallocate called using MEMORY_TAG_UNKNOWN. Re-class this allocation.");
    }

    stats.total_allocated += size;
    stats.tagged_allocations[tag] += size;

    // #todo Memory alignment
    void* block = platform_allocate(size, FALSE);
    platform_zero_memory(block, size);

    return block;
}

void kfree(void* block, u64 size, memory_tag tag) {
    if (tag == MEMORY_TAG_UNKNOWN) {
        KWARN("kfree called using MEMORY_TAG_UNKNOWN. Re-class this allocation.");
    }

    stats.total_allocated -= size;
    stats.tagged_allocations[tag] -= size;

    // #todo Memory alignment
    platform_free(block, FALSE);
}

void* kzero_memory(void* block, u64 size) {
    return platform_zero_memory(block, size);
}

void* kcopy_memory(void* dest, const void* source, u64 size) {
    return platform_copy_memory(dest, source, size);
}

void* kset_memory(void* dest, i32 value, u64 size) {
    return platform_set_memory(dest, value, size);
}

char* get_memory_usage_str() {
    const u64 gib = 1024 * 1024 * 1024;
    const u64 mib = 1024 * 1024;
    const u64 kib = 1024;

    char buffer[8000] = "System memory use (tagged):\n";
    u64 offset = strlen(buffer);
    for (u32 i = 0; i < MEMORY_TAG_MAX_TAGS; ++i) {
        char unit[4] = "XiB";
        f32 amount = 1.0f;  // #note he used 'float' instead of 'f32'
        if (stats.tagged_allocations[i] >= gib) {
            unit[0] = 'G';
            amount = stats.tagged_allocations[i] / (f32)gib;  // #note again, he used 'float'
        } else if (stats.tagged_allocations[i] >= mib) {
            unit[0] = 'M';
            amount = stats.tagged_allocations[i] / (f32)mib;  // #note again, he used 'float'
        } else if (stats.tagged_allocations[i] >= kib) {
            unit[0] = 'K';
            amount = stats.tagged_allocations[i] / (f32)kib;  // #note again, he used 'float'
        } else {
            unit[0] = 'B';
            unit[1] = 0;
            amount = (f32)stats.tagged_allocations[i];  // #note again he used 'float'
        }

        i32 length = snprintf(buffer + offset, 8000, "  %s: %.2f%s\n", memory_tag_strings[i], amount, unit);
        offset += length;
    }

    char* out_string = _strdup(buffer);
    return out_string;
}
