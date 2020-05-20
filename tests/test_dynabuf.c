#include <alibc/extensions/dynabuf.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <setjmp.h>
#include <cmocka.h>

struct test {
    char *a, *b; // 16 bytes
};

static int init(void **state) {
    dynabuf_t *uut = create_dynabuf(3, sizeof(int));
    dynabuf_set(uut, 0, 1);
    dynabuf_set(uut, 1, 2);
    *state = uut;
    return 0;
}

static int init_big(void **state) {
    dynabuf_t *uut = create_dynabuf(2, sizeof(struct test));
    *state = uut;
    return 0;
}

static int finish(void **state) {
    dynabuf_free((dynabuf_t*)*state);
    return 0;
}

static void test_resize(void **state) {
    dynabuf_t *uut = *state;
    dynabuf_resize(uut, 20);
    dynabuf_set(uut, 19, 19);
    assert_int_equal(*(int*)dynabuf_fetch(uut, 19), 19);
}

static void test_set(void **state) {
    dynabuf_t *uut = *state;
    assert_int_equal(*(int*)dynabuf_fetch(uut, 0), 1);
    assert_int_equal(*(int*)dynabuf_fetch(uut, 1), 2);

    dynabuf_set(uut, 0, 3);
    assert_int_equal(*(int*)dynabuf_fetch(uut, 0), 3);
}

static void test_set_big(void **state) {
    dynabuf_t *uut = *state;
    struct test t1 = {.a = "test", .b = "string"};
    struct test t2 = {.a = "large", .b = "block"};
    dynabuf_set(uut, 0, &t1);
    dynabuf_set(uut, 1, &t2);
    struct test r1 = *(struct test*)dynabuf_fetch(uut, 0);
    struct test r2 = *(struct test*)dynabuf_fetch(uut, 1);
    assert_true(strcmp(r1.a, "test") == 0);
    assert_true(strcmp(r1.b, "string") == 0);

    assert_true(strcmp(r2.a, "large") == 0);
    assert_true(strcmp(r2.b, "block") == 0);
}

static void test_set_seq(void **state) {
    dynabuf_t *uut = *state;
    int next = dynabuf_set_seq(uut, 0, 0, "pointer to constant string", sizeof(char*));
    dynabuf_set_seq(uut, 0, next, "second half of insert", sizeof(char*));
    struct test *result = (struct test *)dynabuf_fetch(uut, 0);
    assert_true(strcmp(result->a, "pointer to constant string") == 0);
    assert_true(strcmp(result->b, "second half of insert") == 0);

    // check that invalid next causes a return of -1
    // test size is 16
    // note that set_seq interprets the element pointer based on the size
    // argument, NOT based on target->elem_size.  Thus, we actually copy eight
    // bytes in the following calls, not sixteen. (on a 64 bit system).
    // Neither of the following calls should cause any writes, however.
    next = dynabuf_set_seq(uut, 1, 17, "doesn't matter", sizeof(char*));
    assert_int_equal(next, -1);
    
    next = dynabuf_set_seq(uut, 1, -5, "doesn't matter", sizeof(char*));
    assert_int_equal(next, -1);
}

static void test_set_seq_big(void **state) {
    dynabuf_t *uut = *state;
    // insert an actual string, not a pointer to it.
    int next = dynabuf_set_seq(uut, 0, 0, "hello world", 11);
    // "!test" but encoded in big-endian int format.
    dynabuf_set_seq(uut, 0, next, 0x7473657421l, 5);
    char *result = (char*)dynabuf_fetch(uut, 0);
    assert_true(strcmp(result, "hello world!test") == 0);
}

static void test_invalid_calls(void **state) {
    int r = dynabuf_set(NULL, 0, 1);
    // test the check_valid line for every function
    assert_int_equal(r, ARG_INVAL);

    r = dynabuf_fetch(NULL, 0);
    assert_int_equal(r, NULL); // special case here

    r = dynabuf_set_seq(NULL, 0, 0, 1, sizeof(int));
    assert_int_equal(r, -1); // special case for set_seq

    r = dynabuf_resize(NULL, 0);
    assert_int_equal(r, ARG_INVAL);

    // test NO_MEM in check_valid
    dynabuf_t *uut = *state;
    uut-> capacity = 0;
    r = dynabuf_set(uut, 0, 1);
    assert_int_equal(r, NO_MEM);

    // test NULL_BUF in check_valid
    free(uut->buf);
    uut->buf = NULL;

    r = dynabuf_set(uut, 0, 1);
    assert_int_equal(r, NULL_BUF);

    // destructor to test null target in dynabuf_free
    free(uut);
    *state = NULL;
}


int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(
            test_set,
            init,
            finish
        ),
        cmocka_unit_test_setup_teardown(
            test_resize,
            init,
            finish
        ),
        cmocka_unit_test_setup_teardown(
            test_invalid_calls,
            init,
            finish
        )
    };

    const struct CMUnitTest tests_big[] = {
        cmocka_unit_test_setup_teardown(
            test_set_seq,
            init_big,
            finish
        ),
        cmocka_unit_test_setup_teardown(
            test_set_big,
            init_big,
            finish
        ),
        cmocka_unit_test_setup_teardown(
            test_set_seq_big,
            init_big,
            finish
        ),
    };

    int r = cmocka_run_group_tests(tests, NULL, NULL);
    if(r == 0) {
        r = cmocka_run_group_tests(tests_big, NULL, NULL);
    }

    return r;
}
