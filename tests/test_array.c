#include <stdio.h>
#include <string.h>
#include <alibc/extensions/array.h>
#include <alibc/extensions/iterator.h>
#include <alibc/extensions/array_iterator.h>
#include <criterion/criterion.h>
// ideas: 
// implementation enforcement with macros via IMPLEMENTS(proto)
// inheritance via extension, possibly after the { on a struct
// # define protocol(...) typedef struct {???

/*
 *struct test {
 *    proto_list;
 *    int x;
 *};
 */
char *data[] = {"the quick", "brown fox", "jumped over", "the lazy dog"};
array_t *at_uut;

void at_init(void)  {
    at_uut = create_array(1);
    cr_assert_not_null(at_uut);
    for(int i = 0; i < 4; i++)  {
        array_append(at_uut, data[i]);
    }
}

void at_finish(void)    {
    array_free(at_uut);
}

TestSuite(array_tests, .init=at_init, .fini=at_finish);

Test(array_tests, insert)   {
    for(int i = 0; i < 4; i++)  {
        array_insert(at_uut, i, data[3-i]);
    }
    array_insert(at_uut, 2, "lorem ipsum");
    int size = array_size(at_uut);
    cr_assert_eq(size, 9, "size check failed with size %d", size);
    char *result    = array_fetch(at_uut, 2);
    cr_assert(strcmp(result, "lorem ipsum") == 0,
                "equality check failed, got: %s", result); 
    result  = array_fetch(at_uut, array_size(at_uut)-1);
    // test swap() by proxy
    cr_assert(strcmp(result, "brown fox") == 0,
            "equality check failed, got: %s", result);
}

Test(array_tests, insert_unsafe) {
    for(int i = 0; i < 4; i++)  {
        array_insert(at_uut, i, data[3-i]);
    }
    array_insert_unsafe(at_uut, 2, "lorem ipsum");
    int size = array_size(at_uut);
    cr_assert_eq(size, 8, "size check failed with size %d", size);
    char *result    = array_fetch(at_uut, 2);
    cr_assert(strcmp(result, "lorem ipsum") == 0,
                "equality check failed, got: %s", result); 

    array_insert_unsafe(at_uut, array_size(at_uut)-1, "unsafe insert");
    result  = array_fetch(at_uut, array_size(at_uut)-1);
    // test swap() by proxy
    cr_assert(strcmp(result, "unsafe insert") == 0,
            "equality check failed, got: %s", result);
}

Test(array_tests, append)   {
    for(int i = 0; i < 4; i++)  {
        array_append(at_uut, data[i]);
    }
    int size = array_size(at_uut);
    cr_assert_eq(size, 8, "size check failed with size %d", size);
    char *result    = array_fetch(at_uut, 6);
    cr_assert(strcmp(result, "jumped over") == 0,
                "equality check failed, got: %s", result); 
}

Test(array_tests, fetch)    {
    char *result    = array_fetch(at_uut, 2);
    cr_assert(strcmp(result, "jumped over") == 0,
                "equality check failed, got: %s", result); 
    int size = array_size(at_uut);
    cr_assert_eq(size, 4, "size check failed with size %d", size);
    
    result    = array_fetch(at_uut, 6);
    cr_assert_null(result, "fetch beyond end of array returned non-null");
    
    result    = array_fetch(at_uut, -1);
    cr_assert_null(result, "fetch of negative index returned non-null");
}

Test(array_tests, remove)   {
    char *result    = array_remove(at_uut, 2);
    cr_assert(strcmp(result, "jumped over") == 0,
                "equality check failed, got: %s", result); 
    int size = array_size(at_uut);
    cr_assert_eq(size, 3, "size check failed with size %d", size);
    
    result    = array_fetch(at_uut, 6);
    cr_assert_null(result, "remove beyond end of array returned non-null");
    
    result    = array_fetch(at_uut, -1);
    cr_assert_null(result, "remove of negative index returned non-null");
}

Test(array_tests, resize) {
    int result;
    for(int i = 0; i < 5; i++) {
        array_append(at_uut, i);
    }
    result = array_resize(at_uut, 5);
    cr_assert_eq(result, array_okay(at_uut), "status not set");
    cr_assert_eq(result, IDX_OOB, "Allowed resize of < count of elements.");

    result = array_resize(at_uut, array_size(at_uut));
    cr_assert_eq(result, SUCCESS, "resize no-op failed.");

    result = array_resize(at_uut, 50);
    cr_assert_eq(result, SUCCESS, "could not resize array to size 50.");
}

Test(index_tests, indices) {
    array_t *uut    = create_array(1);
    int result      = array_remove(uut, 0);
    cr_assert_null(result, "empty removal");
    result  = array_fetch(uut, 0);
    cr_assert_null(result, "empty fetch");
    result  = array_remove(uut, 1);
    cr_assert_null(result, "empty remove beyond array limits");
    result  = array_fetch(uut, 1);
    cr_assert_null(result, "empty fetch beyond array limits");

    array_append(uut, (void*)1);
    result  = array_fetch(uut, 0);
    cr_assert_eq(result, 1, "incorrect fetch");
    result  = array_fetch(uut, 1);
    cr_assert_null(result, "fetch beyond array limits");
    result  = array_remove(uut, 1);
    cr_assert_null(result, "remove beyond array limits");
    result  = array_fetch(uut, 0);
    cr_assert_eq(result, 1, "fetch beyond array limits");

    array_free(uut);
}

Test(array_tests, iterator) {
    iter_context *iter = create_array_iterator(at_uut);
    cr_assert_not_null(iter, "iterator was null");
    cr_assert_eq(iter->status, ITER_READY);
    // iterate one too far - check stop
    for(int i = 0; i < 5; i++) {
        char *next = iter_next(iter);
        if(i < 5) {
            cr_assert_eq(iter->status, ITER_CONTINUE);
        }
        else if(i == 5) {
            cr_assert_eq(iter->status, ITER_STOP);
            // oob case
            cr_assert_null(next, "returned a non-null result out of bounds");
        }
    }
    iter_free(iter);
}
