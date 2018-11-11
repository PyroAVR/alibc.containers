#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "include/bitmap.h"
#include <criterion/criterion.h>

bitmap_t *bm_uut;

void bm_init(void) {
    bm_uut = create_bitmap(4);
}

void bm_finish(void) {
    bitmap_free(bm_uut);
}

TestSuite(bitmap_tests, .init=bm_init, .fini=bm_finish, .timeout=0);

Test(bitmap_tests, min_size) {
    cr_assert_eq(bm_uut->capacity, 1, "allocated bad array size");
}

Test(bitmap_tests, add) {
    uint8_t p = 1;
    for(int i = 0; i < 8; i++) {
        bitmap_add(bm_uut, i);
        cr_assert(((char*)bm_uut->buf)[0] & (1 << i),
                "incorrect bitmap value for %d on insert", i);
        p |= p << 1;
    }
}

Test(bitmap_tests, remove) {
    ((char*)bm_uut->buf)[0] = 255;
    int p = 255;
    for(int i = 7; i >= 0; i--) {
        bitmap_remove(bm_uut, i);
        p >>= 1;
        cr_assert_eq(((char*)bm_uut->buf)[0], p, 
                "incorrect bitmap value for %d on remove", i);
    }
}

Test(bitmap_tests, contains) {
    bitmap_add(bm_uut, 1);
    bitmap_add(bm_uut, 1);
    bitmap_add(bm_uut, 2);
    bitmap_add(bm_uut, 3);
    bitmap_add(bm_uut, 5);
    cr_assert(bitmap_contains(bm_uut, 1), "could not find 1");
    cr_assert(bitmap_contains(bm_uut, 2), "could not find 2");
    cr_assert(bitmap_contains(bm_uut, 3), "could not find 3");
    cr_assert(bitmap_contains(bm_uut, 5), "could not find 5");
}

Test(bitmap_tests, resize) {
    bitmap_resize(bm_uut, 65536);
    bitmap_add(bm_uut, 41139);
    cr_assert(bitmap_contains(bm_uut, 41139) > 0, "41139 not in bitmap");
}
