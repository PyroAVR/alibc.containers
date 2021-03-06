#pragma once
#include <alibc/containers/iterator.h>
#include <alibc/containers/set.h>

/**
 * Create a new iterator from the given set, starting at the first element.
 * @param target The set whose elements will be iterated over
 * @return a new iterator context, or NULL on error.
 */
iter_context *create_set_iterator(set_t *target);
