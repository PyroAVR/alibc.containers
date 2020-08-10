#include <alibc/containers/hashmap.h>
#include <alibc/containers/iterator.h>
#include <stdlib.h>
#include <stdio.h>
#define value_at(self, idx) (void**)((char*)dynabuf_fetch(self->map, idx) + self->val_offset)
#define key_at(self, idx) (void**)((char*)dynabuf_fetch(self->map, idx))

static void **hashmap_iter_keys(iter_context *ctx) {
    void **r = NULL;
    hashmap_t *target = (hashmap_t*)ctx->_data;
    if(target == NULL) {
        ctx->status = ALC_ITER_INVALID;
        goto done;
    }
    while(ctx->index < target->capacity
        && !bitmap_contains(target->_filter, ctx->index)) {
        ctx->index++;
    }
    if(bitmap_contains(target->_filter, ctx->index)) {
        r = key_at(target, ctx->index);
    }

    if(ctx->index == target->capacity) {
        ctx->status = ALC_ITER_STOP;
    }
    else {
        ctx->status = ALC_ITER_CONTINUE;
        ctx->index++;
    }
done:
    return r;
}

static void **hashmap_iter_values(iter_context *ctx) {
    void **r = NULL;
    hashmap_t *target = (hashmap_t*)ctx->_data;
    if(target == NULL) {
        ctx->status = ALC_ITER_INVALID;
        goto done;
    }
    while(ctx->index < target->capacity
        && !bitmap_contains(target->_filter, ctx->index)) {
        ctx->index++;
    }
    if(bitmap_contains(target->_filter, ctx->index)) {
        r = value_at(target, ctx->index);
    }

    if(ctx->index == target->capacity) {
        ctx->status = ALC_ITER_STOP;
    }
    else {
        ctx->status = ALC_ITER_CONTINUE;
        ctx->index++;
    }
done:
    return r;
}

iter_context *create_hashmap_keys_iterator(hashmap_t *target) {
    iter_context *r = malloc(sizeof(iter_context));
    if(r == NULL) {
        goto done;
    }
    r->index = 0;
    r->status = ALC_ITER_READY;
    r->_data = target;
    r->next = hashmap_iter_keys;
done:
    return r;
}

iter_context *create_hashmap_values_iterator(hashmap_t *target) {
    iter_context *r = malloc(sizeof(iter_context));
    if(r == NULL) {
        goto done;
    }
    r->index = 0;
    r->status = ALC_ITER_READY;
    r->_data = target;
    r->next = hashmap_iter_values;
done:
    return r;
}
