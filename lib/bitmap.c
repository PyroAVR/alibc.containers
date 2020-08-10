#include <alibc/containers/bitmap.h>
#include <stddef.h>
#include <string.h>

bitmap_t *create_bitmap(int max) {
    // can't have less than one byte allocated.
    int size_in_bytes = (max + 7) >> 3;
    dynabuf_t *buf = create_dynabuf(size_in_bytes, sizeof(char));
    memset(buf->buf, 0, size_in_bytes);
    return buf;
}

bitmap_t *bitmap_resize(bitmap_t *self, int max) {
    // can't have less than one byte allocated.
    int size_in_bytes = (max + 7) >> 3;
    int old_size = self->capacity;
    int status = dynabuf_resize(self, size_in_bytes);
    // zero out the newly allocated chunk. (dynabuf does not use calloc)
    memset((char*)self->buf + old_size, 0, size_in_bytes - old_size);
    return (status  == ALC_DYNABUF_SUCCESS) ? self:NULL;
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
