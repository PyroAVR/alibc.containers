#include <alibc/extensions/hashmap.h>
#include <alibc/extensions/iterator.h>
#include <alibc/extensions/hashmap_iterator.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <criterion/criterion.h>

char *names[] = {"one", "two", "three", "four", "five",
                 "six", "seven", "eight", "nine", "ten"};

hashmap_t *ht_uut;

void ht_init()  {
    ht_uut  = create_hashmap(2, hashmap_hash_i64, strcmp, NULL);
    cr_assert_not_null(ht_uut);
    for(uint64_t i = 0; i < 10; i++) {
        hashmap_set(ht_uut, names[i], (void*)(i + 1));
    }
}

void ht_finish()    {
    hashmap_free(ht_uut);
}

TestSuite(hash_tests, .init=ht_init, .fini=ht_finish);

Test(hash_tests, test_setget)  {
    for(uint64_t i = 0; i < 10; i++)    {
        uint64_t result     = (uint64_t)hashmap_fetch(ht_uut, (void*)names[i]);
        cr_assert_eq(result, i + 1, 
                    "map contains invalid entry for key %s: %d", names[i], result); 
    }

    uint64_t result = (uint64_t)hashmap_fetch(ht_uut, (void*)0);
    cr_assert_null(result, "fetch returned non-null for null key: %d", result);
    cr_assert_eq(hashmap_okay(ht_uut), IDX_OOB, "incorrect status value");
}

Test(hash_tests, test_remove)   {
    for(uint64_t i = 0; i < 10; i++)    {
        uint64_t result = (uint64_t)hashmap_remove(ht_uut, (void*)names[i]);
        cr_assert_eq(result, i + 1, "map contains invalid entry for key %s:%d",
                names[i], result); 
    }
    cr_assert_null(hashmap_remove(ht_uut, "two hundred twenty one"), 
                    "removal of key not in map returned non-null");

    for(uint64_t i = 0; i < 10; i++)    {
        uint64_t result = (uint64_t)hashmap_fetch(ht_uut, (void*)names[i]);
        cr_assert_null(result, "got result: %d for key: %s", result, names[i]);
    }
}

Test(hash_tests, resize) {
    int result;
    for(int i = 0; i < 5; i++) {
        hashmap_set(ht_uut, (void*)names[i], (void*)i);
    }
    result = hashmap_resize(ht_uut, 5);
    cr_assert_eq(result, hashmap_okay(ht_uut), "status not set");
    cr_assert_eq(result, IDX_OOB, "Allowed resize of < count of elements.");

    result = hashmap_resize(ht_uut, hashmap_size(ht_uut));
    cr_assert_eq(result, SUCCESS, "resize no-op failed.");

    result = hashmap_resize(ht_uut, 50);
    cr_assert_eq(result, SUCCESS, "could not resize hashmap to size 50.");
}

Test(hash_tests, test_size) {
    int size = hashmap_size(ht_uut);
    cr_assert_eq(size, 10, "size check failed, got %d", size);
}

Test(hash_tests, iterator_keys) {
    iter_context *iter = create_hashmap_keys_iterator(ht_uut);
    cr_assert_not_null(iter, "iterator was null");
    cr_assert_eq(iter->status, ITER_READY);
    // iterate one too far - check stop
    for(int i = 0; i < 11; i++) {
        char *next = iter_next(iter);
        if(i < 11) {
            cr_assert_eq(iter->status, ITER_CONTINUE);
        }
        else if(i == 11) {
            cr_assert_eq(iter->status, ITER_STOP);
            // oob case
            cr_assert_null(next, "returned a non-null result out of bounds");
        }
    }
    iter_free(iter);
}

Test(hash_tests, iterator_values) {
    iter_context *iter = create_hashmap_values_iterator(ht_uut);
    cr_assert_not_null(iter, "iterator was null");
    cr_assert_eq(iter->status, ITER_READY);
    // iterate one too far - check stop
    for(int i = 0; i < 11; i++) {
        char *next = iter_next(iter);
        if(i < 11) {
            cr_assert_eq(iter->status, ITER_CONTINUE);
        }
        else if(i == 11) {
            cr_assert_eq(iter->status, ITER_STOP);
            // oob case
            cr_assert_null(next, "returned a non-null result out of bounds");
        }
    }
    iter_free(iter);
}
