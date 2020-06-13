#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <alibc/extensions/set.h>
#include <alibc/extensions/hashmap.h>
#include <alibc/extensions/iterator.h>
#include <alibc/extensions/set_iterator.h>
#include <string.h>
#include <setjmp.h>
#include <cmocka.h>

char *items[] = {"do not go gentle into that good night,",
                 "old age should burn and rave at the close of day,",
                 "rage, rage, against the dying of the light.",
                 "though wise men at their end know dark is right,",
                 "because their words had forked no lightning they",
                 "do not go gentle into that good night."
};

static bool full_load(int entries, int capacity) {
    return entries >= capacity;
}

static int set_init(void **state) {
    set_t *uut = create_set(
        1, sizeof(char*), hashmap_hash_str, strcmp, full_load
    );
    *state = uut;
    return 0;
}

static int set_finish(void **state) {
    set_t *uut = *state;
    set_free(uut);
    return 0;
}


static void test_add(void **state) {
    set_t *uut = *state;
    for(int i = 0; i < 6; i++)  {
        set_add(uut, items[i]);
    }
    for(int i = 0; i < 6; i++) {
        assert_true(set_contains(uut, items[i]));
    }

    int result = set_contains(uut, "we live, as we dream... alone.");
    assert_int_equal(result, 0);
    assert_int_equal(set_okay(uut), ALC_SET_NOTFOUND);
    
    // test resize case 1: no space on first addition 
    // (we'll have to corrupt the structure to do this)
    uut->capacity = set_size(uut);
    result = set_add(uut, "resize me");
    assert_int_equal(result, ALC_SET_SUCCESS);
}

static void test_remove(void **state) {
    set_t *uut = *state;
    for(int i = 0; i < 6; i++)  {
        set_add(uut, items[i]);
    }
    for(int i = 0; i < 6; i++) {
        set_remove(uut, items[i]);
        assert_false(set_contains(uut, items[i]));
    }

    // remove a non-existant element
    assert_null(set_remove(uut, "whoops please edit"));
    
    // ensure uniqueness
    set_add(uut, "blep");
    set_add(uut, "blep");
    set_remove(uut, "blep");
    assert_false(set_contains(uut, "blep"));
}

static void test_resize(void **state) {
    set_t *uut = *state;
    int result;
    char *more_items[] = {"one", "two", "three", "four", "five"};
    for(int i = 0; i < 5; i++) {
        set_add(uut, (void*)more_items[i]);
    }
    // do not allow resize to smaller than number of elements
    result = set_resize(uut, 3);
    assert_int_equal(result, set_okay(uut));
    assert_int_equal(result, ALC_SET_INVALID_REQ);

    // ensure no-op is allowed
    result = set_resize(uut, set_size(uut));
    assert_int_equal(result, ALC_SET_SUCCESS);

    // ensure pre-reservation of extra space works
    result = set_resize(uut, 50);
    assert_int_equal(result, ALC_SET_SUCCESS);
}

static void test_iterator(void **state) {
    // test with a bad context
    iter_context *iter = create_set_iterator(NULL);
    char *next = iter_next(iter);
    assert_null(next);
    assert_int_equal(iter_okay(iter), ITER_INVALID);
    iter_free(iter);

    set_t *uut = *state;
    for(int i = 0; i < 6; i++)  {
        set_add(uut, items[i]);
    }
    iter = create_set_iterator(uut);
    assert_non_null(iter);
    assert_int_equal(iter->status, ITER_READY);
    // iterate one too far - check stop
    for(int i = 0; i < set_size(uut) + 1; i++) {
        next = iter_next(iter);
        if(next != NULL) {
            next = *(char**)next;
        }
        if(i < set_size(uut)) {
            assert_int_equal(iter->status, ITER_CONTINUE);
            assert_true(set_contains(uut, next));
        }
        else {
            assert_int_equal(iter->status, ITER_STOP);
            // oob case
            assert_null(next);
        }
    }
    iter_free(iter);
}

static void test_invalid_calls(void **state) {
    // test every check_space_available/check_valid case
    int r = set_add(NULL, NULL);
    assert_int_equal(r, ALC_SET_INVALID);

    r = set_remove(NULL, NULL);
    assert_null(r);

    r = set_resize(NULL, 0);
    assert_int_equal(r, ALC_SET_INVALID);

    r = set_size(NULL);
    assert_int_equal(r, -1);

    r = set_contains(NULL, NULL);
    assert_int_equal(r, 0);

    // check for invalid load function
    set_t *uut = *state;
    uut->load = NULL;
    assert_int_equal(set_add(uut, NULL), ALC_SET_INVALID);

    // invalid compare function
    uut->compare = NULL;
    assert_int_equal(set_add(uut, NULL), ALC_SET_INVALID);

    // invalid bitmap
    uut->_filter = NULL;
    assert_int_equal(set_add(uut, NULL), ALC_SET_INVALID);

    // invalid buffer
    uut->buf = NULL;
    assert_int_equal(set_add(uut, NULL), ALC_SET_INVALID);
}

static int bad_hash_fn(void *item) {
    if(item == NULL) return 0;
    char *real_item = (char*)item;
    int sum = 0;
    for(int i = 0; i < strlen(real_item); i++) {
        sum += real_item[i];
    }
    return sum;
}

static void test_locate(void **state) {
    set_t *uut = *state;
    uut->compare = bad_hash_fn;
    for(int i = 0; i < 6; i++)  {
        set_add(uut, items[i]);
    }
    set_add(uut, NULL);
    assert_true(set_contains(uut, NULL));
    assert_false(set_contains(uut, "ef"));
}

int main(int argc, char **argv) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(
            test_add,
            set_init,
            set_finish
        ),
        cmocka_unit_test_setup_teardown(
            test_remove,
            set_init,
            set_finish
        ),
        cmocka_unit_test_setup_teardown(
            test_resize,
            set_init,
            set_finish
        ),
        cmocka_unit_test_setup_teardown(
            test_iterator,
            set_init,
            set_finish
        ),
        cmocka_unit_test_setup_teardown(
            test_invalid_calls,
            set_init,
            set_finish
        ),
        cmocka_unit_test_setup_teardown(
            test_locate,
            set_init,
            set_finish
        )
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
