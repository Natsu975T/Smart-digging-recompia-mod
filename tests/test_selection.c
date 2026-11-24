#include "selection.h"

#include <assert.h>
#include <limits.h>
#include <stdio.h>

static void test_requires_two_points(void) {
    SdSelection s;
    SdRegion r;
    sd_selection_reset(&s);
    assert(!sd_selection_build_region(&s, 1000, &r));
    assert(!sd_selection_set_point_b(&s, (SdCell){1, 2, 3}));
}

static void test_two_point_cuboid_and_iteration(void) {
    SdSelection s;
    SdRegion r;
    SdCell c;
    sd_selection_reset(&s);
    assert(sd_selection_set_point_a(&s, (SdCell){4, 10, 7}));
    assert(sd_selection_set_point_b(&s, (SdCell){2, 99, 5}));
    assert(sd_selection_set_depth(&s, 3));
    assert(sd_selection_build_region(&s, 100, &r));
    assert(r.min_x == 2 && r.max_x == 4);
    assert(r.min_z == 5 && r.max_z == 7);
    assert(r.top_y == 10 && r.min_y == 8);
    assert(r.block_count == 27);
    assert(sd_region_cell_at(&r, 0, &c) && c.x == 2 && c.y == 10 && c.z == 5);
    assert(sd_region_cell_at(&r, 2, &c) && c.x == 4 && c.y == 10 && c.z == 5);
    assert(sd_region_cell_at(&r, 3, &c) && c.x == 2 && c.y == 10 && c.z == 6);
    assert(sd_region_cell_at(&r, 9, &c) && c.x == 2 && c.y == 9 && c.z == 5);
    assert(sd_region_cell_at(&r, 26, &c) && c.x == 4 && c.y == 8 && c.z == 7);
    assert(!sd_region_cell_at(&r, 27, &c));
}

static void test_rock_smash_batches_max_four(void) {
    SdSelection s;
    SdRegion r;
    SdCell batch[SD_ROCK_SMASH_BATCH_MAX];

    sd_selection_reset(&s);
    assert(sd_selection_set_point_a(&s, (SdCell){0, 10, 0}));
    assert(sd_selection_set_point_b(&s, (SdCell){4, 10, 0}));
    assert(sd_selection_set_depth(&s, 2));
    assert(sd_selection_build_region(&s, 100, &r));
    assert(r.block_count == 10);

    assert(sd_region_batch_size(&r, 0) == 4);
    assert(sd_region_batch_size(&r, 4) == 4);
    assert(sd_region_batch_size(&r, 8) == 2);
    assert(sd_region_batch_size(&r, 10) == 0);
    assert(sd_region_batch_size(&r, 100) == 0);

    assert(sd_region_batch_at(&r, 0, batch) == 4);
    assert(batch[0].x == 0 && batch[3].x == 3);
    assert(sd_region_batch_at(&r, 4, batch) == 4);
    assert(batch[0].x == 4 && batch[3].y == 9);
    assert(sd_region_batch_at(&r, 8, batch) == 2);
    assert(batch[0].x == 3 && batch[0].y == 9);
    assert(batch[1].x == 4 && batch[1].y == 9);
    assert(sd_region_batch_at(&r, 10, batch) == 0);
}

static void test_depth_and_reset(void) {
    SdSelection s;
    SdRegion r;
    sd_selection_reset(&s);
    assert(sd_selection_set_point_a(&s, (SdCell){-2, 0, -3}));
    assert(sd_selection_set_point_b(&s, (SdCell){-2, 0, -3}));
    assert(sd_selection_build_region(&s, 10, &r) && r.block_count == 1 && r.min_y == 0);
    assert(!sd_selection_set_depth(&s, 0));
    assert(!sd_selection_set_depth(&s, 257));
    assert(sd_selection_set_depth(&s, 4));
    assert(sd_selection_build_region(&s, 10, &r) && r.block_count == 4 && r.min_y == -3);
    sd_selection_reset(&s);
    assert(s.depth == 1 && !s.has_point_a && !s.has_point_b);
}

static void test_limits_and_overflow(void) {
    SdSelection s;
    SdRegion r;
    sd_selection_reset(&s);
    assert(sd_selection_set_point_a(&s, (SdCell){INT32_MIN, INT32_MIN, 0}));
    assert(sd_selection_set_point_b(&s, (SdCell){INT32_MAX, INT32_MIN, 0}));
    assert(!sd_selection_build_region(&s, 4096, &r));
    assert(sd_selection_set_point_a(&s, (SdCell){0, INT32_MIN, 0}));
    assert(sd_selection_set_point_b(&s, (SdCell){0, INT32_MIN, 0}));
    assert(sd_selection_set_depth(&s, 2));
    assert(!sd_selection_build_region(&s, 4096, &r));
}

int main(void) {
    test_requires_two_points();
    test_two_point_cuboid_and_iteration();
    test_rock_smash_batches_max_four();
    test_depth_and_reset();
    test_limits_and_overflow();
    puts("selection tests: ok");
    return 0;
}
