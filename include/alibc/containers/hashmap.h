#pragma once
#include <alibc/containers/dynabuf.h> 
#include <alibc/containers/bitmap.h>
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>

/**
 * alibc/containers hashmap interface
 * A compact open-addressing hash-backed key-value map.
 * Guarantees:
 *  - entry validity
 *  - entry uniqueness
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
    int    entries;
    int    capacity;
    int    status;
    int val_offset;
} hashmap_t;

typedef enum {
    ALC_HASHMAP_SUCCESS = 0,
    ALC_HASHMAP_INVALID = INT_MIN,
    ALC_HASHMAP_INVALID_REQ,
    ALC_HASHMAP_NOTFOUND,
    ALC_HASHMAP_NO_MEM,
    ALC_HASHMAP_FAILURE
} hashmap_error_t;

/*
 * Constructor function for hashmap type
 * @param size the starting size of the map
 * @param keysz size of keys, in bytes.
 * @param valsz size of values in bytes.
 * @param hashfn the hash function to use for this map
 * @param comparefn the comparator to use for this map
 * @param loadfn memory load estimator, used to reduce collisions.
 * @return new hashmap, or null or errors
 */
hashmap_t *create_hashmap(int size, int keysz, int valsz, hash_type *hashfn,
        cmp_type *comparefn, load_type loadfn);

/*
 * Add a new key-value pair to the map
 * @param self the map to use
 * @param key the key which will be used to fetch the value
 * @param value the value which will be associated with the given key
 * @return hashmap_error_t error code
 */
int hashmap_set(hashmap_t *self, void *key, void *value);

/*
 * Retrieve the value associated with the given key
 * @param self the map to use
 * @param key the key to find in the map
 * @return pointer to the associated value, or NULL if the key is not known.
 */
void **hashmap_fetch(hashmap_t *self, void *key);

/*
 * Forget the association between a key and its value.
 * @param self the map to use
 * @param key the key whose value should be forgotten
 * @return pointer to the value, or NULL if the key is not known.
 */
void **hashmap_remove(hashmap_t *self, void *key);

/*
 * Allocate sufficient space for count items to be stored in the hashmap.
 * If the count is greater than the number of entries, but less than the number
 * of items which are currently allocated, shrink (with copy) to a new buffer.
 * @param self the hashmap to resize
 * @param count the number of entries which should be allocated.
 * @return hashmap_error_t error code.
 */
int hashmap_resize(hashmap_t *self, int count);

/*
 * Compute the size in entries of the hashmap
 * @param self the map to use
 * @return the size of the map in elements. 
 */
int hashmap_size(hashmap_t *self);

/*
 * Return the memory used to allocate the hashmap and underlying buffers to the
 * system.  The hashmap should be considered invalid after this operation.
 * @param self the hashmap to free
 */
void hashmap_free(hashmap_t *self);

/*
 * Ascertain the status of the previous operation
 * @param self the hashmap to validate
 * @return hashmap_error_t error code from the previous operation 
 */
int hashmap_status(hashmap_t *self);
