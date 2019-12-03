#pragma once
#include <stdint.h>
/*
 * Iterator interface for alc data structures.
 * Each iterator is self-contained, meaning that several contexts may
 * instantiate iterators over the same instance of a structure.
 * It is considered safe to use iterators only when the underlying data is
 * not being changed.  Consult the documentation for each data structure's
 * implementation for information on MT-safety.
 */

typedef struct _iter_context iter_context;
typedef void **(iter_next_fn)(iter_context *ctx);

struct _iter_context {
    uint32_t index;
    uint32_t status;
    void *_data;
    iter_next_fn *next;
};

typedef enum {
    ITER_READY,
    ITER_CONTINUE,
    ITER_INVALID,
    ITER_NULL,
    ITER_STOP
} iter_status;

/**
 * Retrieve the next element from a previously created iterator.
 * @param ctx The iterator context which should be used to fetch and update
 * @return The next object in the sequence, or zero on error. Use iter_okay for
 * result validity checking.
 */
void **iter_next(iter_context *ctx);

/**
 * Retrieve the status of an iterator.
 * @param ctx The iterator to determine the status of
 * @return A value in iter_status.
 */
int iter_okay(iter_context *ctx);

/**
 * Free the memory associated with an iterator.  Does not affect the underlying
 * data structure.
 * @param ctx the iterator to free
 * @return none
 */
void iter_free(iter_context *ctx);
