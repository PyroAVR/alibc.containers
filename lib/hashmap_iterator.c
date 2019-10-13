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
    for(; ctx->index < target->capacity; ctx->index++) {
        if(bitmap_contains(target->_filter, ctx->index)) {
            r = ((kv_pair*)target->map->buf)[ctx->index].key;
            break;
        }
    }
    if(ctx->index == target->capacity) {
        ctx->status = ITER_STOP;
    }
    else {
        ctx->status = ITER_CONTINUE;
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
    for(; ctx->index < target->capacity; ctx->index++) {
        if(bitmap_contains(target->_filter, ctx->index)) {
            r = ((kv_pair*)target->map->buf)[ctx->index].value;
            break;
        }
    }
    if(ctx->index == target->capacity) {
        ctx->status = ITER_STOP;
    }
    else {
        ctx->status = ITER_CONTINUE;
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
