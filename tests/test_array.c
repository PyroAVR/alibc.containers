#include <stdio.h>
#include <string.h>
#include "include/array.h"
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
    cr_assert(strcmp(result, "the lazy dog") == 0,
                "equality check failed, got: %s", result); 
    int size = array_size(at_uut);
    cr_assert_eq(size, 3, "size check failed with size %d", size);
    
    result    = array_fetch(at_uut, 6);
    cr_assert_null(result, "remove beyond end of array returned non-null");
    
    result    = array_fetch(at_uut, -1);
    cr_assert_null(result, "remove of negative index returned non-null");
}
