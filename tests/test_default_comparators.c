#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>
#include <cmocka.h>
#include <alibc/containers/comparable.h>
#include <alibc/containers/comparators.h>

static void test_cmp_i8(void **state) {
    assert_true(alc_default_cmp_i8(0, 0) == 0);
    assert_true(alc_default_cmp_i8(127, 127) == 0);
    assert_true(alc_default_cmp_i8(-5, -1) < 0);
    assert_true(alc_default_cmp_i8(-1, -5) > 0);
    assert_true(alc_default_cmp_i8(1, 5) < 0);
    assert_true(alc_default_cmp_i8(5, 1) > 0);
}

static void test_cmp_u8(void **state) {
    assert_true(alc_default_cmp_u8(0, 0) == 0);
    assert_true(alc_default_cmp_u8(127, 127) == 0);
    assert_true(alc_default_cmp_u8(-5, -1) < 0);
    assert_true(alc_default_cmp_u8(-1, -5) > 0);
    assert_true(alc_default_cmp_u8(1, 5) < 0);
    assert_true(alc_default_cmp_u8(5, 1) > 0);
}

static void test_cmp_i16(void **state) {
    assert_true(alc_default_cmp_i16(0, 0) == 0);
    assert_true(alc_default_cmp_i16(127, 127) == 0);
    assert_true(alc_default_cmp_i16(-5, -1) < 0);
    assert_true(alc_default_cmp_i16(-1, -5) > 0);
    assert_true(alc_default_cmp_i16(1, 5) < 0);
    assert_true(alc_default_cmp_i16(5, 1) > 0);
}

static void test_cmp_u16(void **state) {
    assert_true(alc_default_cmp_u16(0, 0) == 0);
    assert_true(alc_default_cmp_u16(127, 127) == 0);
    assert_true(alc_default_cmp_u16(-5, -1) < 0);
    assert_true(alc_default_cmp_u16(-1, -5) > 0);
    assert_true(alc_default_cmp_u16(1, 5) < 0);
    assert_true(alc_default_cmp_u16(5, 1) > 0);
}
static void test_cmp_i32(void **state) {
    assert_true(alc_default_cmp_i32(0, 0) == 0);
    assert_true(alc_default_cmp_i32(127, 127) == 0);
    assert_true(alc_default_cmp_i32(-5, -1) < 0);
    assert_true(alc_default_cmp_i32(-1, -5) > 0);
    assert_true(alc_default_cmp_i32(1, 5) < 0);
    assert_true(alc_default_cmp_i32(5, 1) > 0);
}

static void test_cmp_u32(void **state) {
    assert_true(alc_default_cmp_u32(0, 0) == 0);
    assert_true(alc_default_cmp_u32(127, 127) == 0);
    assert_true(alc_default_cmp_u32(-5, -1) < 0);
    assert_true(alc_default_cmp_u32(-1, -5) > 0);
    assert_true(alc_default_cmp_u32(1, 5) < 0);
    assert_true(alc_default_cmp_u32(5, 1) > 0);
}
static void test_cmp_i64(void **state) {
    assert_true(alc_default_cmp_i64(0, 0) == 0);
    assert_true(alc_default_cmp_i64(127, 127) == 0);
    assert_true(alc_default_cmp_i64(-5, -1) < 0);
    assert_true(alc_default_cmp_i64(-1, -5) > 0);
    assert_true(alc_default_cmp_i64(1, 5) < 0);
    assert_true(alc_default_cmp_i64(5, 1) > 0);
}

static void test_cmp_u64(void **state) {
    assert_true(alc_default_cmp_u64(0, 0) == 0);
    assert_true(alc_default_cmp_u64(127, 127) == 0);
    assert_true(alc_default_cmp_u64(-5, -1) < 0);
    assert_true(alc_default_cmp_u64(-1, -5) > 0);
    assert_true(alc_default_cmp_u64(1, 5) < 0);
    assert_true(alc_default_cmp_u64(5, 1) > 0);
}

static void test_cmp_ptr(void **state) {
    assert_true(
        alc_default_cmp_ptr(0x01020304050607, 0x0100020304050607) < 0
    );
}

static void test_cmp_str(void **state) {
    assert_true(
            alc_default_cmp_str("abcdefh", "abcdefg") > 0
    );
    assert_true(alc_default_cmp_str("def", "ef") < 0);
}
int main(int argc, char **argv) {
    static struct CMUnitTest tests[] = {
        cmocka_unit_test(test_cmp_i8),
        cmocka_unit_test(test_cmp_u8),
        cmocka_unit_test(test_cmp_i16),
        cmocka_unit_test(test_cmp_u16),
        cmocka_unit_test(test_cmp_i32),
        cmocka_unit_test(test_cmp_u32),
        cmocka_unit_test(test_cmp_i64),
        cmocka_unit_test(test_cmp_u64),
        cmocka_unit_test(test_cmp_ptr),
        cmocka_unit_test(test_cmp_str),

    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
