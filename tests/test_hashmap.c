#include <alibc/containers/hashmap.h>
#include <alibc/containers/hash_functions.h>
#include <alibc/containers/comparators.h>
#include <alibc/containers/iterator.h>
#include <alibc/containers/hashmap_iterator.h>
#include <alibc/containers/bitmap.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <setjmp.h>
#include <cmocka.h>

char *names[] = {"one", "two", "three", "four", "five",
                 "six", "seven", "eight", "nine", "ten"};

static int ht_init(void **state)  {
    hashmap_t *uut  = create_hashmap(
        2, sizeof(char*), sizeof(int), alc_default_hash_i64, strcmp, NULL
    );
    assert_non_null(uut);
    for(uint64_t i = 0; i < 10; i++) {
        hashmap_set(uut, names[i], (void*)(i + 1));
    }
    *state = uut;
    return 0;
}

static int ht_finish(void **state)    {
    hashmap_t *uut = *state;
    hashmap_free(uut);
    return 0;
}

static bool full_load(int entries, int capacity) {
    return entries >= capacity;
}

static void test_set_get(void **state) {
    hashmap_t *uut = *state;
    uut->load = full_load;
    for(int i = 0; i < 10; i++)    {
        int result     = *(int*)hashmap_fetch(uut, (void*)names[i]);
        assert_int_equal(result, i + 1);
    }

    // check that null key returns null
    uint64_t result = (uint64_t)hashmap_fetch(uut, (void*)0);
    assert_null(result);
    assert_int_equal(hashmap_status(uut), ALC_HASHMAP_NOTFOUND);

    // check that an unmapped key returns null
    result = (uint64_t)hashmap_fetch(uut, "two twenty one");
    assert_null(result);

    // test key-uniqueness and overwrite-priority
    hashmap_set(uut, "eleven", 12);
    hashmap_set(uut, "eleven", 11);
    result = hashmap_fetch(uut, "eleven");
    assert_non_null(result);
    assert_int_equal(*(int*)result, 11);

    // test null key
    hashmap_set(uut, NULL, 221);
    result = hashmap_fetch(uut, NULL);
    assert_non_null(result);
    assert_int_equal(*(int*)result, 221); 

    // test no-mem rehash (structure corruption is required)
    uut->capacity = hashmap_size(uut);
    hashmap_set(uut, "rehash", 1);
}

static void test_remove(void **state) {
    hashmap_t *uut = *state;
    int result;
    for(int i = 0; i < 10; i++)    {
        result = *(int*)hashmap_remove(uut, (void*)names[i]);
        assert_int_equal(result, i + 1);
    }
    assert_null(hashmap_remove(uut, "two hundred twenty one"));

    for(int i = 0; i < 10; i++) {
        // careful not to deref null...
        result = (int*)hashmap_fetch(uut, (void*)names[i]);
        assert_null(result);
    }
}

static void test_resize(void **state) {
    hashmap_t *uut = *state;
    int result;
    for(int i = 0; i < 5; i++) {
        hashmap_set(uut, (void*)names[i], (void*)i);
    }
    result = hashmap_resize(uut, 5);
    assert_int_equal(result, hashmap_status(uut));
    assert_int_equal(result, ALC_HASHMAP_INVALID_REQ);

    result = hashmap_resize(uut, hashmap_size(uut));
    assert_int_equal(result, ALC_HASHMAP_SUCCESS);

    result = hashmap_resize(uut, 50);
    assert_int_equal(result, ALC_HASHMAP_SUCCESS);
}

static void test_size(void **state) {
    hashmap_t *uut = *state;
    int size = hashmap_size(uut);
    assert_int_equal(size, 10);
}

static void test_iter_keys(void **state) {
    // test with an invalid iterator
    iter_context *iter = create_hashmap_keys_iterator(NULL);
    char *next = iter_next(iter);
    assert_null(next);
    assert_int_equal(iter_status(iter), ALC_ITER_INVALID);
    iter_free(iter);

    hashmap_t *uut = *state;
    iter = create_hashmap_keys_iterator(uut);
    assert_non_null(iter);
    assert_int_equal(iter->status, ALC_ITER_READY);
    // iterate one too far - check stop
    for(int i = 0; i < 11; i++) {
        next = iter_next(iter);
        if(i < 10) {
            assert_int_equal(iter->status, ALC_ITER_CONTINUE);
        }
        else if(i == 10) {
            assert_int_equal(iter->status, ALC_ITER_STOP);
            // oob case
            assert_null(next);
        }
    }
    iter_free(iter);
}

static void test_iter_values(void **state) {
    // test with an invalid iterator
    iter_context *iter = create_hashmap_values_iterator(NULL);
    char *next = iter_next(iter);
    assert_null(next);
    assert_int_equal(iter_status(iter), ALC_ITER_INVALID);
    iter_free(iter);

    hashmap_t *uut = *state;
    iter = create_hashmap_values_iterator(uut);
    assert_non_null(iter);
    assert_int_equal(iter->status, ALC_ITER_READY);
    bitmap_t *values_bmp = create_bitmap(sizeof(names)/sizeof(char*));
    // iterate one too far - check stop
    for(int i = 0; i < 11; i++) {
        next = (int*)iter_next(iter);
        if(i < 10) {
            assert_int_equal(iter->status, ALC_ITER_CONTINUE);
            bitmap_add(values_bmp, *next);
        }
        else if(i == 10) {
            assert_int_equal(iter->status, ALC_ITER_STOP);
            // oob case
            assert_null(next);
        }
    }
    for(int i = 0; i < sizeof(names)/sizeof(char*); i++) {
        assert_true(bitmap_contains(values_bmp, i + 1));
    }
    iter_free(iter);
}

static void test_invalid_calls(void **state) {
    // test all check_valid/check_space_available calls with NULL self
    int r = hashmap_set(NULL, "key", 0);
    assert_int_equal(r, ALC_HASHMAP_FAILURE);

    r = hashmap_resize(NULL, 0);
    assert_int_equal(r, ALC_HASHMAP_FAILURE);

    r = hashmap_remove(NULL, "key");
    assert_null(r);

    r = hashmap_size(NULL);
    assert_int_equal(r, -1);

    r = hashmap_fetch(NULL, "key");
    assert_null(r);

    // test null bitmap
    hashmap_t *uut = *state;
    bitmap_free(uut->_filter);
    uut->_filter = NULL;
    r = hashmap_set(uut, "key", 0);
    assert_int_equal(r, ALC_HASHMAP_INVALID);
    // test load
    uut->load = NULL;
    r = hashmap_set(uut, "key", 0);
    assert_int_equal(r, ALC_HASHMAP_INVALID);

    // test hash
    uut->hash = NULL;
    r = hashmap_set(uut, "key", 0);
    assert_int_equal(r, ALC_HASHMAP_INVALID);

    // test compare
    uut->compare = NULL;
    r = hashmap_set(uut, "key", 0);
    assert_int_equal(r, ALC_HASHMAP_INVALID);

    // test buf
    dynabuf_free(uut->map);
    uut->map = NULL;
    r = hashmap_set(uut, "key", 0);
    assert_int_equal(r, ALC_HASHMAP_INVALID);
}

int main(int argc, char **argv) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(
            test_set_get,
            ht_init,
            ht_finish
        ),
        cmocka_unit_test_setup_teardown(
            test_resize,
            ht_init,
            ht_finish
        ),
        cmocka_unit_test_setup_teardown(
            test_remove,
            ht_init,
            ht_finish
        ),
        cmocka_unit_test_setup_teardown(
            test_iter_keys,
            ht_init,
            ht_finish
        ),
        cmocka_unit_test_setup_teardown(
            test_iter_values,
            ht_init,
            ht_finish
        ),
        cmocka_unit_test_setup_teardown(
            test_size,
            ht_init,
            ht_finish
        ),
        cmocka_unit_test_setup_teardown(
            test_invalid_calls,
            ht_init,
            ht_finish
        )
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
