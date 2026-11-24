#ifndef SMART_DIGGING_SELECTION_H
#define SMART_DIGGING_SELECTION_H

#include <stdbool.h>
#include <stdint.h>

typedef struct SdCell {
    int32_t x;
    int32_t y;
    int32_t z;
} SdCell;

typedef struct SdSelection {
    SdCell point_a;
    SdCell point_b;
    uint32_t depth;
    bool has_point_a;
    bool has_point_b;
} SdSelection;

typedef struct SdRegion {
    int32_t min_x;
    int32_t max_x;
    int32_t min_y;
    int32_t top_y;
    int32_t min_z;
    int32_t max_z;
    uint64_t block_count;
} SdRegion;

/* Point A defines the top Y plane. Point B defines the opposite X/Z corner. */
void sd_selection_reset(SdSelection *selection);
bool sd_selection_set_point_a(SdSelection *selection, SdCell cell);
bool sd_selection_set_point_b(SdSelection *selection, SdCell cell);
bool sd_selection_set_depth(SdSelection *selection, uint32_t depth);
bool sd_selection_build_region(const SdSelection *selection, uint64_t max_blocks, SdRegion *out);
/* Stable iteration order: X fastest, then Z, then Y from top to bottom. */
bool sd_region_cell_at(const SdRegion *region, uint64_t index, SdCell *out);

#endif
