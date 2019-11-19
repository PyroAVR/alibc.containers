#pragma once
#include "errors.h"

/**
 * Dynamically sized memory allocations, with variable (but not dynamic) element
 * size.
 *  if the element size is <= sizeof(void*), we assume the element is a pointer
 *  or a value which is smaller than a pointer, and so write it directly.
 *  else, we take the value and assume it is a pointer to a memory location of
 *  the element size, and copy it.
 */

typedef struct {
    char *buf;
    int capacity;
    int elem_size;
} dynabuf_t;

typedef alibc_internal_errors dynabuf_status;

/**
 * Create a new dynabuf, with a given size, and an allocation unit size.
 * When accessed using "dynabuf_fetch" and "dynabuf_set", this is the size of
 * memory which will be used.
 * @param size the size of the buffer to create, in elements.
 * @param unit the size of each element, in bytes.
 * @return dynabuf_t, or NULL on error.
 */
dynabuf_t *create_dynabuf(int size, int unit);

/**
 * Insert a new element in the dynabuf.
 * @param target the dynabuf to write to
 * @param which the location to put the element in.
 * @param element a pointer to the element which should be written.
 * @return alibc_internal_errors compliant.
 */
int dynabuf_set(dynabuf_t *target, int which, void *element);


/**
 * Fetch an element from the dynabuf.
 * @param target the dynabuf to fetch from
 * @param idx the element number to retrieve
 * @return the element, or NULL on error.
 */
void *dynabuf_fetch(dynabuf_t *target, int which);

/**
 * Resize the dynabuf to a certain size, in elements.
 * @param target the dynabuf whose backing buffer should be resized
 * @param count the number of elements which should be present.
 * @return alibc_internal_errors compliant.
 */
dynabuf_status dynabuf_resize(dynabuf_t *target, int count);
/**
 * Free the memory associated with a particular dynabuf.
 * @param target the dynabuf to free
 */
void dynabuf_free(dynabuf_t *target);

