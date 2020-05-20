#include <alibc/extensions/set.h>
#include <alibc/extensions/iterator.h>
#include <alibc/extensions/dynabuf.h>
#include <stdlib.h>
#include <stdio.h>

static void **set_iter_next(iter_context *ctx) {
    void **r = NULL;
    set_t *target = (set_t*)ctx->_data;
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
        r = dynabuf_fetch(target->buf, ctx->index);
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
    r->status = ITER_READY;
    r->_data = target;
    r->next = set_iter_next;
done:
    return r;
}
