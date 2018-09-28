#include "map.h"
#include "list.h"
#include <stdint.h>

typedef uint32_t (hash_type)(void *);
typedef uint32_t (load_type)(proto_map *map);
typedef uint8_t (cmp_type)(void *, void*);

proto_map *create_hashmap(hash_type *hashfn);

void apply_custom_hash(proto_map *map, hash_type *func);

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
