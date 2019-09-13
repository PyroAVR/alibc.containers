#pragma once
#include <stdint.h>
#include "dynabuf.h"
/**
 * alibc/extensions array interface
 * A simple contiguous-allocation array.
 * Guarantees:
 *  - contiguous allocation
 *  - Constant fetch/append time
 *  - Constant insert/remove time (possibly changing to log?)
 *  - Memory-safe - objects stored must be deallocated on their own.
 * Non-Guarantees:
 *  - Ordering.  The array is not ordered, insertion and removal will change
 *    the order of stored objects not inserted or removed unless the operation
 *    decays to fetch/append.
 */

/*
 * array type definition
 */
typedef struct  {
    dynabuf_t   *data;
    uint32_t    size;
    uint32_t    status;
} array_t;

/*
 * array status indication - compliant with alibc/extensions standard errors
 */
// FIXME
typedef alibc_internal_errors array_status;

/*
 * Constructor function for array type
 * @param size the size to reserve.
 * @return new array, or null on errors.
 */
array_t *create_array(uint32_t size);

/*
 * Insert a new value into the array
 * @param self array to insert into
 * @param where the location at which to insert
 * @param item the object to store
 * @return insert status
 */
array_status array_insert(array_t *self, int where, void* item);

/*
 * Append a new value to the end of the array
 * @param self the array to append to
 * @param item the item to append
 * @return append status
 */
array_status array_append(array_t *self, void* item);

/*
 * Insert a new value into the array without swapping, thus overwriting
 * any previously stored value at that index.
 * @param self the array to insert into
 * @param where the index to which item shall be written
 * @param item the value to store at the specified index.
 * @return insert status
 */
array_status array_insert_unsafe(array_t *self, int where, void *item);

/*
 * Retrieve an item from the array
 * @param self the array from which to retrieve the item
 * @param which the index to fetch
 * @return the item, or NULL if it does not exist.
 */
void *array_fetch(array_t *self, int which);

/*
 * Allocate space for at least count items.
 * If count is less than the current capacity and greater than or equal to the
 * current size, then constrain the array (with copy) to the new, smaller size.
 * @param self the array to use
 * @param count the number of items which should be able to fit in the array.
 * @return array_status_code
 */
int array_resize(array_t *self, int count);

/*
 * Remove an item from the array
 * @param self the array from which to remove the item
 * @param which the index to remove
 * @return the item, or NULL if it does not exist.
 */
void *array_remove(array_t *self, int which);

/*
 * Cause the index of two objects in the array to be exchanged
 * @param self the array to adjust
 * @param first the index of the first object to swap
 * @param second the index of the second object to swap
 * @return status of the swap
 */
array_status array_swap(array_t *self, int first, int second);

/*
 * Compute the size of the array, in entries.  The size in bytes is
 * the platform bit-width multiplied by this size in entries.
 * @param self the array to compute the size of
 * @return the size, -1 on error.
 */
int array_size(array_t *self);

/*
 * Return the memory used to allocate the array and underlying buffers to the
 * system.  The array should be considered invalid after this operation.
 * @param self the array to free
 */
void array_free(array_t *self);

/*
 * Ascertain the status of the previous operation
 * @param self the array to validate
 * @return the alibc/extensions error code, zero if successful.
 */
int array_okay(array_t *self);
