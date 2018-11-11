#pragma once
#include "dynabuf.h"
/*
 * Linear Bitmap
 * Compact set type for dense, integer valued sets.
 * Not especially useful for large integers unless scaling is applied and the
 * expected numbers are in a dense group.
 * While resizing is supported, it is not recommended, for the same
 * reasons as above if being used as a set.  If being used as a filter/marker,
 * then this is acceptable usage.
 */

typedef dynabuf_t bitmap_t;

/*
 * Constructor function for bitmaps.
 * Functionally equivalent to the dynabuf constructor.
 * @param max the new max-value to store in the set.
 * @return the new bitmap, or NULL on errors.
 */
bitmap_t *create_bitmap(int size);

/*
 * Resize the bitmap to hold a maximum value of a given size.
 * @param self the bitmap to use
 * @param max the new max-value to store in the set.
 * @return pointer to the resized set, or NULL on error.
 */
bitmap_t *bitmap_resize(bitmap_t *self, int max);

/*
 * Check for the existence of key in the given bitmap
 * @param self the bitmap to use
 * @param key the key to find in the set
 * @return non-zero value for true, else zero.
 */
int bitmap_contains(bitmap_t *self, int key);

/*
 * Insert a key into the given bitmap
 * @param self the bitmap to use
 * @param key the key which should be added
 */
void bitmap_add(bitmap_t *self, int key);

/*
 * Zero-out the entry at key in the given bitmap
 * @param self the bitmap to use
 * @param key the key which should be forgotten
 */
void bitmap_remove(bitmap_t *self, int key);

/*
 * Destroy the target bitmap
 * @param self the bitmap to use
 */
void bitmap_free(bitmap_t *self);
