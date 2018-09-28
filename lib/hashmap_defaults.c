#include "../include/hashmap.h"
#include "../include/debug.h"


uint8_t hashmap_cmp_i8(void *a, void *b);
uint8_t hashmap_cmp_i16(void *a, void *b);
uint8_t hashmap_cmp_i32(void *a, void *b);
uint8_t hashmap_cmp_i64(void *a, void *b);
uint8_t hashmap_cmp_ptr(void *a, void *b);
uint8_t hashmap_cmp_str(void *a, void *b);
uint32_t hashmap_hash_i8(void *val);
uint32_t hashmap_hash_i16(void *val);
uint32_t hashmap_hash_i32(void *val);
uint32_t hashmap_hash_i64(void *val);
uint32_t hashmap_hash_ptr(void *ptr);
uint32_t hashmap_hash_str(void *str);
