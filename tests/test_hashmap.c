#include "include/hashmap.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <criterion/criterion.h>

char *names[] = {"one", "two", "three", "four", "five",
                 "six", "seven", "eight", "nine", "ten"};

hashmap_t *ht_uut;

void ht_init()  {
    ht_uut  = create_hashmap(hashmap_hash_i64, hashmap_cmp_str, 2);
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
}

Test(hash_tests, test_remove)   {
    for(uint64_t i = 0; i < 10; i++)    {
        uint64_t result = (uint64_t)hashmap_remove(ht_uut, (void*)names[i]);
        cr_assert_eq(result, i + 1, "map contains invalid entry for key %s:%d",
                names[i], result); 
    }
    for(uint64_t i = 0; i < 10; i++)    {
        uint64_t result = (uint64_t)hashmap_fetch(ht_uut, (void*)names[i]);
        cr_assert_null(result, "got result: %d for key: %s", result, names[i]);
    }
}

Test(hash_tests, test_size) {
    int size = hashmap_size(ht_uut);
    cr_assert_eq(size, 10, "size check failed, got %d", size);
}
