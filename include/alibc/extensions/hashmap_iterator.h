#pragma once
#include <alibc/extensions/iterator.h>
#include <alibc/extensions/hashmap.h>

/**
 * Create a new iterator from the given hashmap, starting at the first element.
 * The keys contained in the map are returned.
 * @param target The hashmap whose elements will be iterated over
 * @return a new iterator context, or NULL on error.
 */
iter_context *create_hashmap_keys_iterator(hashmap_t *target);


/**
 * Create a new iterator from the given hashmap, starting at the first element.
 * The values contained in the map are returned.
 * @param target The hashmap whose elements will be iterated over
 * @return a new iterator context, or NULL on error.
 */
iter_context *create_hashmap_values_iterator(hashmap_t *target);
