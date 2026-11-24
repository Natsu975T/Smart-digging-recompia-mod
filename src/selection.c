#include "selection.h"

#include <limits.h>
#include <stddef.h>
#include <string.h>

void sd_selection_reset(SdSelection *selection) {
    if (selection) {
        memset(selection, 0, sizeof(*selection));
        selection->depth = 1;
    }
}

bool sd_selection_set_point_a(SdSelection *selection, SdCell cell) {
    if (!selection) return false;
    selection->point_a = cell;
    selection->has_point_a = true;
    selection->has_point_b = false;
    return true;
}

bool sd_selection_set_point_b(SdSelection *selection, SdCell cell) {
    if (!selection || !selection->has_point_a) return false;
    selection->point_b = cell;
    selection->has_point_b = true;
    return true;
}

bool sd_selection_set_depth(SdSelection *selection, uint32_t depth) {
    if (!selection || depth == 0 || depth > 256) return false;
    selection->depth = depth;
    return true;
}

static uint64_t span_i32(int32_t lo, int32_t hi) {
    return (uint64_t)((int64_t)hi - (int64_t)lo) + 1u;
}

bool sd_selection_build_region(const SdSelection *selection, uint64_t max_blocks, SdRegion *out) {
    if (!selection || !out || !selection->has_point_a || !selection->has_point_b ||
        selection->depth == 0 || selection->depth > 256 || max_blocks == 0) return false;

    const int32_t min_x = selection->point_a.x < selection->point_b.x ? selection->point_a.x : selection->point_b.x;
    const int32_t max_x = selection->point_a.x > selection->point_b.x ? selection->point_a.x : selection->point_b.x;
    const int32_t min_z = selection->point_a.z < selection->point_b.z ? selection->point_a.z : selection->point_b.z;
    const int32_t max_z = selection->point_a.z > selection->point_b.z ? selection->point_a.z : selection->point_b.z;
    const int64_t min_y64 = (int64_t)selection->point_a.y - ((int64_t)selection->depth - 1);
    if (min_y64 < INT32_MIN || min_y64 > INT32_MAX) return false;

    const uint64_t width = span_i32(min_x, max_x);
    const uint64_t length = span_i32(min_z, max_z);
    const uint64_t height = selection->depth;
    if (width > max_blocks || length > max_blocks || height > max_blocks) return false;
    if (width > max_blocks / length) return false;
    const uint64_t footprint = width * length;
    if (footprint > max_blocks / height) return false;
    const uint64_t count = footprint * height;

    out->min_x = min_x;
    out->max_x = max_x;
    out->min_y = (int32_t)min_y64;
    out->top_y = selection->point_a.y;
    out->min_z = min_z;
    out->max_z = max_z;
    out->block_count = count;
    return true;
}

bool sd_region_cell_at(const SdRegion *region, uint64_t index, SdCell *out) {
    if (!region || !out || region->block_count == 0 || index >= region->block_count) return false;
    const uint64_t width = span_i32(region->min_x, region->max_x);
    const uint64_t length = span_i32(region->min_z, region->max_z);
    const uint64_t plane = width * length;
    const uint64_t y_offset = index / plane;
    const uint64_t within_plane = index % plane;
    const uint64_t z_offset = within_plane / width;
    const uint64_t x_offset = within_plane % width;
    const int64_t y = (int64_t)region->top_y - (int64_t)y_offset;
    const int64_t z = (int64_t)region->min_z + (int64_t)z_offset;
    const int64_t x = (int64_t)region->min_x + (int64_t)x_offset;
    if (x > INT32_MAX || z > INT32_MAX || y < INT32_MIN || y > INT32_MAX) return false;
    out->x = (int32_t)x;
    out->y = (int32_t)y;
    out->z = (int32_t)z;
    return true;
}


uint32_t sd_region_batch_size(const SdRegion *region, uint64_t first_index) {
    if (!region || first_index >= region->block_count) return 0;
    const uint64_t remaining = region->block_count - first_index;
    return (uint32_t)(remaining < SD_ROCK_SMASH_BATCH_MAX ? remaining : SD_ROCK_SMASH_BATCH_MAX);
}

uint32_t sd_region_batch_at(const SdRegion *region, uint64_t first_index,
                            SdCell out[SD_ROCK_SMASH_BATCH_MAX]) {
    if (!out) return 0;
    const uint32_t count = sd_region_batch_size(region, first_index);
    for (uint32_t i = 0; i < count; ++i) {
        if (!sd_region_cell_at(region, first_index + i, &out[i])) return 0;
    }
    return count;
}
