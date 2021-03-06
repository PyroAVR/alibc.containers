#include <alibc/containers/set.h>
#include <alibc/containers/iterator.h>
#include <alibc/containers/dynabuf.h>
#include <stdlib.h>
#include <stdio.h>

static void **set_iter_next(iter_context *ctx) {
    void **r = NULL;
    set_t *target = (set_t*)ctx->_data;
    if(target == NULL) {
        ctx->status = ALC_ITER_INVALID;
        goto done;
    }
    while(ctx->index < target->capacity
       && !bitmap_contains(target->_filter, ctx->index)) {
        ctx->index++;
    }
    if(bitmap_contains(target->_filter, ctx->index)) {
        r = dynabuf_fetch(target->buf, ctx->index);
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

iter_context *create_set_iterator(set_t *target) {
    iter_context *r = malloc(sizeof(iter_context));
    if(r == NULL) {
        goto done;
    }
    r->index = 0;
    r->status = ALC_ITER_READY;
    r->_data = target;
    r->next = set_iter_next;
done:
    return r;
}
