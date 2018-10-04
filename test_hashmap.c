#include "include/map.h"
#include "include/hashmap.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

char *names[] = {"one", "two", "three", "four", "five",
                 "six", "seven", "eight", "nine", "ten"};

int main(void)  {
    proto_map *uut  = create_hashmap(hashmap_hash_i64, hashmap_cmp_str);    
    for(uint64_t i = 0; i < 10; i++) {
        uut->set(uut, names[i], (void*)i);
    }
    for(uint64_t i = 0; i < 10; i++)    {
        printf("k: %s v: %d\n", names[i], (uint64_t)uut->fetch(uut, (void*)names[i])); 
    }
    return 0;
}
