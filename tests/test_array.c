#include <stdio.h>
#include <string.h>
#include <alibc/extensions/array.h>
#include <alibc/extensions/iterator.h>
#include <alibc/extensions/array_iterator.h>
#include <stdlib.h>
#include <setjmp.h>
#include <cmocka.h>


char *data[] = {"the quick", "brown fox", "jumped over", "the lazy dog"};

static int at_init(void **state) {
    array_t *at_uut = create_array(1, sizeof(char*));
    assert_non_null(at_uut);
    for(int i = 0; i < 4; i++) {
        array_append(at_uut, data[i]);
    }
    *state = at_uut;
    return 0;
}

static int at_finish(void **state) {
    array_t *at_uut = *state;
    array_free(at_uut);
    return 0;
}

// for big item tests
struct test {
    char *a, *b;
};

static int at_init_big(void **state)  {
    array_t *at_uut = create_array(1, sizeof(struct test));
    assert_non_null(at_uut);
    *state = at_uut;
    return 0;
}

static void test_insert(void **state) {
    array_t *at_uut = *state;
    for(int i = 0; i < 4; i++)  {
        array_insert(at_uut, i, data[3-i]);
    }
    array_insert(at_uut, 2, "lorem ipsum");
    int size = array_size(at_uut);
    assert_int_equal(size, 9);
    
    char *result    = *array_fetch(at_uut, 2);
    assert_true(strcmp(result, "lorem ipsum") == 0);

    result  = *array_fetch(at_uut, array_size(at_uut)-1);
    // test swap() by proxy
    assert_true(strcmp(result, "brown fox") == 0);

    // attempt insertion past end of array
    int r = array_insert(at_uut, 20, "not valid");
    assert_int_equal(r, ALC_ARRAY_IDX_OOB);
}

static void test_unsafe_insert(void **state) {
    array_t *at_uut = *state;
    for(int i = 0; i < 4; i++)  {
        array_insert(at_uut, i, data[3-i]);
    }
    array_insert_unsafe(at_uut, 2, "lorem ipsum");
    int size = array_size(at_uut);
    assert_int_equal(size, 8);

    char *result    = *array_fetch(at_uut, 2);
    assert_true(strcmp(result, "lorem ipsum") == 0);

    array_insert_unsafe(at_uut, array_size(at_uut)-1, "unsafe insert");
    result  = *array_fetch(at_uut, array_size(at_uut)-1);
    // test swap() by proxy
    assert_true(strcmp(result, "unsafe insert") == 0);

    // attempt insertion past end of array
    int r = array_insert_unsafe(at_uut, 20, "not valid");
    assert_int_equal(r, ALC_ARRAY_IDX_OOB);
}

static void test_append(void **state) {
    array_t *at_uut = *state;
    for(int i = 0; i < 4; i++)  {
        array_append(at_uut, data[i]);
    }
    int size = array_size(at_uut);
    assert_int_equal(size, 8);

    char *result    = *array_fetch(at_uut, 6);
    assert_true(strcmp(result, "jumped over") == 0);
}

static void test_fetch(void **state) {
    array_t *at_uut = *state;
    char *result    = *array_fetch(at_uut, 2);
    assert_true(strcmp(result, "jumped over") == 0);

    int size = array_size(at_uut);
    assert_int_equal(size, 4);
    
    result    = array_fetch(at_uut, 6);
    assert_null(result);
    
    result    = array_fetch(at_uut, -1);
    assert_null(result);
}

static void test_remove(void **state) {
    array_t *at_uut = *state;
    char *result    = *array_remove(at_uut, 2);
    assert_true(strcmp(result, "jumped over") == 0);

    int size = array_size(at_uut);
    assert_int_equal(size, 3);
    
    result    = array_fetch(at_uut, 6);
    assert_null(result);
    
    result    = array_fetch(at_uut, -1);
    assert_null(result);
}

static void test_resize(void **state) {
    array_t *at_uut = *state;
    int result;
    for(int i = 0; i < 5; i++) {
        array_append(at_uut, i);
    }
    result = array_resize(at_uut, 5);
    // check that array_okay returns the same as the previous status
    assert_int_equal(result, array_okay(at_uut));
    assert_int_equal(result, ALC_ARRAY_IDX_OOB);

    // container shrinking is currently unimplemented, so we test only for no-op
    result = array_resize(at_uut, array_size(at_uut));
    assert_int_equal(result, ALC_ARRAY_SUCCESS);

    result = array_resize(at_uut, 50);
    assert_int_equal(result, ALC_ARRAY_SUCCESS);
}

static void test_indices(void **state) {
    array_t *uut    = create_array(1, sizeof(int));
    int result      = array_remove(uut, 0);
    assert_null(result);
    result  = array_fetch(uut, 0);
    assert_null(result);

    result  = array_remove(uut, 1);
    assert_null(result);

    result  = array_fetch(uut, 1);
    assert_null(result);

    array_append(uut, (void*)1);
    result  = *array_fetch(uut, 0);
    assert_int_equal(result, 1);

    result  = array_fetch(uut, 1);
    assert_null(result);

    result  = array_remove(uut, 1);
    assert_null(result);

    result  = array_remove(uut, 0);
    result  = array_fetch(uut, 0);
    assert_null(result);

    array_free(uut);
}

static void test_iterator(void **state) {
    // test null target calls
    iter_context *iter = create_array_iterator(NULL);
    char *next = iter_next(iter);
    assert_null(next);

    next = iter_next(NULL);
    assert_null(next);
    assert_int_equal(iter_okay(NULL), ITER_NULL);

    array_t *at_uut = *state;
    iter = create_array_iterator(at_uut);
    assert_non_null(iter);
    assert_int_equal(iter->status, ITER_READY);
    // iterate one too far - check stop
    for(int i = 0; i < array_size(at_uut) + 1; i++) {
        next = iter_next(iter);
        // ITER_STOP comes into effect AFTER the last element is pulled
        if(i < array_size(at_uut)-1) {
            assert_int_equal(iter->status, ITER_CONTINUE);
        }
        else if(i == array_size(at_uut)) {
            assert_int_equal(iter_okay(iter), ITER_STOP);
            // oob case
            assert_null(next);
        }
    }
    iter_free(iter);
}

static void test_invalid_calls(void **state) {
    // test  all check_valid/check_space_available cases
    int r = array_insert(NULL, 0, NULL);
    assert_int_equal(r, ALC_ARRAY_INVALID);

    r = array_insert_unsafe(NULL, 0, NULL);
    assert_int_equal(r, ALC_ARRAY_INVALID);

    r = array_append(NULL, 0);
    assert_int_equal(r, ALC_ARRAY_INVALID);

    r = array_fetch(NULL, 0);
    assert_int_equal(r, 0);

    r = array_resize(NULL, 0);
    assert_int_equal(r, ALC_ARRAY_INVALID);

    r = array_remove(NULL, 0);
    assert_int_equal(r, 0);

    r = array_swap(NULL, 0, 1);
    assert_int_equal(r, ALC_ARRAY_INVALID);

    r = array_size(NULL);
    assert_int_equal(r, -1);

    // check NULL_BUF
    array_t *uut = *state;
    dynabuf_free(uut->data);
    uut->data = NULL;

    r = array_size(NULL);
    assert_int_equal(r, -1);

    // check NO_MEM
    uut->size = 0;
    array_append(uut, 1);

    // destructor to test null self
    free(uut);
    *state = NULL;
}

static void test_swap(void **state) {
    // the only case that isn't covered is swapping out of bounds
    array_t *uut = *state;
    int r = array_swap(uut, 100, 0);
    assert_int_equal(r, ALC_ARRAY_IDX_OOB);
}

static void test_swap_big(void **state) {
    array_t *uut = *state;
    struct test t1 = {.a = "str1", .b = "str2"};
    struct test t2 = {.a = "str3", .b = "str4"};
    array_append(uut, &t1);
    array_append(uut, &t2);
    array_swap(uut, 0, 1);
    struct test *r1 = (struct test*)array_fetch(uut, 0);
    struct test *r2 = (struct test*)array_fetch(uut, 1);
    assert_true(strcmp(r1->a, t2.a) == 0);
    assert_true(strcmp(r1->b, t2.b) == 0);
    assert_true(strcmp(r2->a, t1.a) == 0);
    assert_true(strcmp(r2->b, t1.b) == 0);
}


int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(
            test_append,
            at_init,
            at_finish
        ),
        cmocka_unit_test_setup_teardown(
            test_fetch,
            at_init,
            at_finish
        ),
        cmocka_unit_test_setup_teardown(
            test_indices,
            at_init,
            at_finish
        ),
        cmocka_unit_test_setup_teardown(
            test_insert,
            at_init,
            at_finish
        ),
        cmocka_unit_test_setup_teardown(
            test_iterator,
            at_init,
            at_finish
        ),
        cmocka_unit_test_setup_teardown(
            test_remove,
            at_init,
            at_finish
        ),
        cmocka_unit_test_setup_teardown(
            test_resize,
            at_init,
            at_finish
        ),
        cmocka_unit_test_setup_teardown(
            test_unsafe_insert,
            at_init,
            at_finish
        ),
        cmocka_unit_test_setup_teardown(
            test_swap,
            at_init,
            at_finish
        ),
        cmocka_unit_test_setup_teardown(
            test_invalid_calls,
            at_init,
            at_finish
        ),
        // Note: using large-allocation initializer here
        cmocka_unit_test_setup_teardown(
            test_swap_big,
            at_init_big,
            at_finish
        )
    };

    int r = cmocka_run_group_tests(tests, NULL, NULL);
    return r;
}
