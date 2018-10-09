#include "errors.h"
#include "dynabuf.h"
#include <stdint.h>


typedef uint32_t (hash_type)(void *);
/*
 *typedef uint32_t (load_type)(hashmap_t *map);
 */
typedef int8_t (cmp_type)(void *, void*);

typedef struct  {
    dynabuf_t *map;
    hash_type *hash;
    /*
     *load_type *load;
     */
    cmp_type  *compare;
    uint32_t entries;
    uint32_t capacity;
} hashmap_t;

typedef struct {
    void *key;
    void *value;
} kv_pair;
typedef alibc_internal_errors hashmap_status;

hashmap_t *create_hashmap(hash_type *hashfn, cmp_type *comparefn);
hashmap_status hashmap_set(hashmap_t *self, void *key, void *value);
void *hashmap_fetch(hashmap_t *self, void *key);
void *hashmap_remove(hashmap_t *self, void *key);
uint32_t hashmap_size(hashmap_t *self);
void hashmap_free(hashmap_t *self);

void apply_custom_hash(hashmap_t *map, hash_type *func);

int8_t hashmap_cmp_i8(void *a, void *b);
int8_t hashmap_cmp_i16(void *a, void *b);
int8_t hashmap_cmp_i32(void *a, void *b);
int8_t hashmap_cmp_i64(void *a, void *b);
int8_t hashmap_cmp_ptr(void *a, void *b);
int8_t hashmap_cmp_str(void *a, void *b);
uint32_t hashmap_hash_i8(void *val);
uint32_t hashmap_hash_i16(void *val);
uint32_t hashmap_hash_i32(void *val);
uint32_t hashmap_hash_i64(void *val);
uint32_t hashmap_hash_ptr(void *ptr);
uint32_t hashmap_hash_str(void *str);
