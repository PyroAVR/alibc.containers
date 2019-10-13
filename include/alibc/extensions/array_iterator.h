#pragma once
#include <alibc/extensions/iterator.h>
#include <alibc/extensions/array.h>

/**
 * Create a new iterator from the given array, starting at the first element.
 * @param target The array whose elements will be iterated over
 * @return a new iterator context, or NULL on error.
 */
iter_context *create_array_iterator(array_t *target);
