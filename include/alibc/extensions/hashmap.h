#pragma once
#include "errors.h"
#include "dynabuf.h"
#include "bitmap.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * alibc/extensions hashmap interface
 * A compact open-addressing hash-backed key-value map.
 * Guarantees:
 *  - entry validity
 * Non-Guarantees:
 */

/*
 * function typedefs
 */
typedef uint32_t (hash_type)(void *);
typedef int8_t (cmp_type)(void *, void*);
/*
 * Load function type.
 */
typedef bool (load_type)(int entries, int capacity);

/*
 * hashmap type definition
 * _filter is an internal array needed for space-efficiently marking the
 * validity of each entry without using NULL entries or pointers which could
 * cause confusion in the case of a NULL entry being intentional.
 */
typedef struct {
    dynabuf_t *map;
    bitmap_t *_filter;
    hash_type *hash;
    load_type *load;
    cmp_type    *compare;
    uint32_t    entries;
    uint32_t    capacity;
    uint32_t    status;
} hashmap_t;


/*
 * key-value pair definiton - each kv_pair is one entry in the map
 */
typedef struct {
    void *key;
    void *value;
} kv_pair;

/*
 * hashmap status indication - compliant with alibc/extensions standard errors
 */
typedef alibc_internal_errors hashmap_status;

/*
 * Constructor function for hashmap type
 * @param size the starting size of the map
 * @param hashfn the hash function to use for this map
 * @param comparefn the comparator to use for this map
 * @param loadfn memory load estimator, used to reduce collisions.
 * @return new hashmap, or null or errors
 */
hashmap_t *create_hashmap(int size, hash_type *hashfn,
        cmp_type *comparefn, load_type loadfn);

/*
 * Add a new key-value pair to the map
 * @param self the map to use
 * @param key the key which will be used to fetch the value
 * @param value the value which will be associated with the given key
 * @return operation status
 */
hashmap_status hashmap_set(hashmap_t *self, void *key, void *value);

/*
 * Retrieve the value associated with the given key
 * @param self the map to use
 * @param key the key to find in the map
 * @return the associated value, or NULL if the key is not known.
 */
void *hashmap_fetch(hashmap_t *self, void *key);

/*
 * Forget the association between a key and its value.
 * @param self the map to use
 * @param key the key whose value should be forgotten
 * @return the value, or NULL if the key is not known.
 */
void *hashmap_remove(hashmap_t *self, void *key);

/*
 * Allocate sufficient space for count items to be stored in the hashmap.
 * If the count is greater than the number of entries, but less than the number
 * of items which are currently allocated, shrink (with copy) to a new buffer.
 * @param self the hashmap to resize
 * @param count the number of entries which should be allocated.
 * @return hashmap_status_code
 */
int hashmap_resize(hashmap_t *self, int count);

/*
 * Compute the size in entries of the hashmap
 * @param self the map to use
 * @return the size of the map in kv_pair entries. The size in bytes may be
 * obtained by multiplying the result by sizeof(kv_pair)
 */
uint32_t hashmap_size(hashmap_t *self);

/*
 * Return the memory used to allocate the hashmap and underlying buffers to the
 * system.  The hashmap should be considered invalid after this operation.
 * @param self the hashmap to free
 */
void hashmap_free(hashmap_t *self);

/*
 * Ascertain the status of the previous operation
 * @param self the hashmap to validate
 * @return the alibc/extensions error code, zero if successful.
 */
int hashmap_okay(hashmap_t *self);

/*
 * Comparison functions provided as sensible defaults.  Note that the
 * definitions for these functions must be linked separately from 
 * hashmap_defaults.
 */
int8_t hashmap_cmp_i8(void *a, void *b);
int8_t hashmap_cmp_i16(void *a, void *b);
int8_t hashmap_cmp_i32(void *a, void *b);
int8_t hashmap_cmp_i64(void *a, void *b);
int8_t hashmap_cmp_ptr(void *a, void *b);
int8_t hashmap_cmp_str(void *a, void *b);

/*
 * Hash functions provided as sensible defaults.  Note that the definitions
 * for these functions must be linked separately from hashmap_defaults.
 */
uint32_t hashmap_hash_i8(void *val);
uint32_t hashmap_hash_i16(void *val);
uint32_t hashmap_hash_i32(void *val);
uint32_t hashmap_hash_i64(void *val);
uint32_t hashmap_hash_ptr(void *ptr);
uint32_t hashmap_hash_str(void *str);
