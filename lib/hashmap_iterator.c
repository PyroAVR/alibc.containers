#include <alibc/extensions/hashmap.h>
#include <alibc/extensions/iterator.h>
#include <stdlib.h>
#include <stdio.h>
static void *hashmap_iter_keys(iter_context *ctx) {
    void *r = NULL;
    if(ctx == NULL) {
        goto done;
    }
    hashmap_t *target = (hashmap_t*)ctx->_data;
    if(target == NULL) {
        ctx->status = ITER_INVALID;
        goto done;
    }
    while(ctx->index < target->capacity
        && !bitmap_contains(target->_filter, ctx->index)) {
        ctx->index++;
    }
    if(ctx->index == target->capacity) {
        ctx->status = ITER_STOP;
    }
    else {
        ctx->status = ITER_CONTINUE;
        r = ((kv_pair*)dynabuf_fetch(target->map, ctx->index))->key;
    }
done:
    return r;
}

static void *hashmap_iter_values(iter_context *ctx) {
    void *r = NULL;
    if(ctx == NULL) {
        goto done;
    }
    hashmap_t *target = (hashmap_t*)ctx->_data;
    if(target == NULL) {
        ctx->status = ITER_INVALID;
        goto done;
    }
    while(ctx->index < target->capacity
        && !bitmap_contains(target->_filter, ctx->index)) {
        ctx->index++;
    }
    if(ctx->index == target->capacity) {
        ctx->status = ITER_STOP;
    }
    else {
        ctx->status = ITER_CONTINUE;
        r = ((kv_pair*)dynabuf_fetch(target->map, ctx->index))->value;
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
    r->status = ITER_READY;
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
    r->status = ITER_READY;
    r->_data = target;
    r->next = hashmap_iter_values;
done:
    return r;
}
