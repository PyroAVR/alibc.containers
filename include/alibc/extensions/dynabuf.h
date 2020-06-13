#pragma once
#include <limits.h>

/**
 * Dynamically sized memory allocations, with variable (but not dynamic) element
 * size.
 * Each element is accepted as a value whose meaning is interpreted based upon
 * the element size.  Elements with sizeof(element) <= sizeof(void *) will be
 * treated as values.  Elements with sizeof(element) > sizeof(void *) will be
 * treated as pointers to values.
 *
 * A double-pointer is always returned, regardless of the element size.
 */

typedef struct {
    char *buf;
    int capacity;
    int elem_size;
} dynabuf_t;

typedef enum {
    ALC_DYNABUF_SUCCESS = 0,
    ALC_DYNABUF_NO_MEM = INT_MIN,
    ALC_DYNABUF_INVALID
} dynabuf_error_t;

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
 * @return dynabuf_error_t error code.
 */
int dynabuf_set(dynabuf_t *target, int which, void *element);


/**
 * Repeatedly write into the same key, allowing multiple values to be written in
 * sequence.
 * Example use case: storing a key value pair, when the key and value are passed
 * as discontinuous values to a function.
 * @param target the dynabuf to write to
 * @param which the location to put the sequence of items into.
 * @param next the offset to start at for this write within a transaction.
 * @param value the next value to write into the specified key.
 * @param size the size of the value which is to be written next.
 * @return the next offset, or zero when the element is full. -1 on failure.
 *
 * Example usage:
 * int next = dynabuf_set_seq(target, index, 0, "hello world", 11);
 * dynabuf_set_seq(target, index, next, 1, sizeof(int));
 */
int dynabuf_set_seq(dynabuf_t *target, int which, int next,
        void *value, int size);


/**
 * Fetch an element from the dynabuf.
 * @param target the dynabuf to fetch from
 * @param idx the element number to retrieve
 * @return the element, or NULL on error.
 */
void **dynabuf_fetch(dynabuf_t *target, int which);

/**
 * Resize the dynabuf to a certain size, in elements.
 * @param target the dynabuf whose backing buffer should be resized
 * @param count the number of elements which should be present.
 * @return dynabuf_error_t error code.
 */
int dynabuf_resize(dynabuf_t *target, int count);
/**
 * Free the memory associated with a particular dynabuf.
 * @param target the dynabuf to free
 */
void dynabuf_free(dynabuf_t *target);

