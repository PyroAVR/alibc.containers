#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <alibc/extensions/bitmap.h>
#include <setjmp.h>
#include <cmocka.h>

static int bm_init(void **state) {
    bitmap_t *bm_uut = create_bitmap(4);
    *state = bm_uut;
    return 0;
}

static int bm_finish(void **state) {
    bitmap_t *uut = *state;
    bitmap_free(uut);
    return 0;
}

static void test_min_capacity(void **state) {
    bitmap_t *uut = create_bitmap(1);
    assert_int_equal(uut->capacity, 1);
}

static void test_add(void **state) {
    bitmap_t *uut = *state;
    uint8_t p = 1;
    for(int i = 0; i < 8; i++) {
        bitmap_add(uut, i);
        assert_true(((char*)uut->buf)[0] & (1 << i));
        p |= p << 1;
    }
}

static void test_remove(void **state) {
    bitmap_t *uut = *state;
    ((char*)uut->buf)[0] = 255;
    int p = 255;
    for(int i = 7; i >= 0; i--) {
        bitmap_remove(uut, i);
        p >>= 1;
        assert_int_equal(((char*)uut->buf)[0], p);
    }
}

static void test_resize(void **state) {
    bitmap_t *uut = *state;
    uut = bitmap_resize(uut, 10);
    assert_non_null(uut);
    assert_int_equal(uut->capacity, 2);
}

static void test_contains(void **state) {
    bitmap_t *uut = *state;
    bitmap_add(uut, 1);
    bitmap_add(uut, 1);
    bitmap_add(uut, 2);
    bitmap_add(uut, 3);
    bitmap_add(uut, 5);
    assert_true(bitmap_contains(uut, 1));
    assert_true(bitmap_contains(uut, 2));
    assert_true(bitmap_contains(uut, 3));
    assert_true(bitmap_contains(uut, 5));
}

int main(int argc, char **argv) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_min_capacity),
        cmocka_unit_test_setup_teardown(
            test_add,
            bm_init,
            bm_finish
        ),
        cmocka_unit_test_setup_teardown(
            test_remove,
            bm_init,
            bm_finish
        ),
        cmocka_unit_test_setup_teardown(
            test_contains,
            bm_init,
            bm_finish
        ),
        cmocka_unit_test_setup_teardown(
            test_resize,
            bm_init,
            bm_finish
        )
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
