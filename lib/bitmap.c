#include <alibc/extensions/bitmap.h>
#include <stdlib.h>

bitmap_t *create_bitmap(int max) {
    // can't have less than one byte allocated.
    int size_in_bytes = (max + 7) >> 3;
    /*
     *int size_in_bytes = (max > 8) ? ((max + 7) >> 3):1;
     */
    return create_dynabuf(size_in_bytes, sizeof(char));
}

bitmap_t *bitmap_resize(bitmap_t *self, int max) {
    // can't have less than one byte allocated.
    int size_in_bytes = (max > 8) ? (max >> 3):1;
    return (dynabuf_resize(self, size_in_bytes) == SUCCESS) ? self:NULL;
}

int bitmap_contains(bitmap_t *self, int key) {
    int byte_index  = key >> 3;
    int bit_index   = key % 8;
    return ((char*)self->buf)[byte_index] & (1 << bit_index);
}
void bitmap_add(bitmap_t *self, int key) {
    int byte_index  = key >> 3;
    int bit_index   = key % 8;
    ((char*)self->buf)[byte_index] |= (1 << bit_index);
}
void bitmap_remove(bitmap_t *self, int key) {
    int byte_index  = key >> 3;
    int bit_index   = key % 8;
    ((char*)self->buf)[byte_index] &= ~(1 << bit_index);
}

void bitmap_free(bitmap_t *self) {
    dynabuf_free(self);
}
