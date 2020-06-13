#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>
#include <stdbool.h>
#include "dynabuf.h"
#include "bitmap.h"
/*
 * Simple containment set
 * The basic set interface is defined here, more complex set operations can be
 * located in include/set_math.h
 */

/*
 * function typedefs
 */
// gross, FIXME! (alc-24)
typedef uint32_t (hash_type)(void*);
typedef int8_t (cmp_type)(void*, void*);
typedef bool (load_type)(int entries, int capacity);


// load function?
typedef struct {
    dynabuf_t   *buf;
    bitmap_t    *_filter;
    hash_type   *hash;
    load_type *load;
    cmp_type  *compare;
    uint32_t  entries;
    uint32_t  capacity;
    uint32_t  status;
} set_t;

/*
 * Error types for set
 */
typedef enum {
    ALC_SET_SUCCESS = 0,
    ALC_SET_FAILURE = INT_MIN,
    ALC_SET_NOTFOUND,
    ALC_SET_NO_MEM,
    ALC_SET_INVALID,
    ALC_SET_INVALID_REQ
} set_error;

/*
 * Constructor function for set type
 * @param size the initial size to allocate
 * @param unit the size of each set element.
 * @param hashfn hash function to use for items added to the set
 * @param comparefn comparator to check for object equality
 * @param loadfn memory load estimator, for use to reduce collisions.
 * @return pointer to a new set on the heap, or NULL on errors.
 */
set_t *create_set(int size, int unit, hash_type *hashfn,
        cmp_type *comparefn, load_type loadfn);

/*
 * Resize the set to contain at most count items
 * @param self the set to resize
 * @param count hte number of items that should be within the set.
 */
//set_t *set_resize(set_t *self, int count);
/*
 * Add a new member to the set
 * @param self the set to add to 
 * @param item the item to add
 * @return set_status error code.
 */
int set_add(set_t *self, void *item);

/* Remove an existing item from the set
 * The item must be in the set to begin with.
 * @param self the set to use
 * @param item the item which should be dropped from the set
 * @return the item, or NULL on failure.
 */
void **set_remove(set_t *self, void *item);

/*
 * Determine if an item is contained within the set
 * @param self the set to use
 * @param item the item to find in the set
 * @return 1 if the item is found, 0 if not. A set_status will be returned
 * in any error case.
 */
int set_contains(set_t *self, void *item);

/*
 * Resize the set to have count elements allocated.
 * If count is less than the allocated size, but greater than the number of
 * stored elemnets, shrink (with copy) the set to a new buffer.
 * @param self the set to resize
 * @param count the number of elements which this set should be able to hold.
 * @return set_status;
 */
int set_resize(set_t *self, int count);

/* Retrieve an array-like object from the set which can be iterated over.
 * @param self the set which should be iterated over
 * @param context an iteration_context which should be used to store
 * implementation-specific data on, which can be used to retrieve the state of
 * iteration on subsequent calls.
 * @return the next object in the set, or NULL if there are no objects
 * remaining.  The iteration_context will have its status set to ITER_DONE in
 * this case.
 */
/*
 *void *set_iterate(set_t *self, iter_context *context);
 */

/* Get the number of elements contained within the set
 * @param self the set to use
 * @return the number of entries, or -1 on failure.
 */
int set_size(set_t *self);

/*
 * Return the status of the most recent set operation.
 * @param self the set to evaluate
 */
int set_okay(set_t *self);

/*
 * Destroy the set and free all memory allocated by it.
 * @param self the set to destroy
 */
void set_free(set_t *self);
