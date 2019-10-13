#include <alibc/extensions/array_iterator.h>
#include <alibc/extensions/array.h>
#include <stdlib.h>

static void *array_iter_next(iter_context *ctx) {
    void *r = NULL;
    if(ctx == NULL) {
        goto done;
    }
    array_t *target = (array_t*)ctx->_data;
    if(target == NULL) {
        ctx->status = ITER_INVALID;
        goto done;
    }
    for(; ctx->index < array_size(target); ctx->index++) {
        r = array_fetch(target, ctx->index);
        break;
    }
    if(ctx->index == array_size(target) - 1) {
        ctx->status = ITER_STOP;
    }
    else {
        ctx->status = ITER_CONTINUE;
    }
done:
    return r;
}

iter_context *create_array_iterator(array_t *target) {
    iter_context *r = malloc(sizeof(iter_context));
    if(r == NULL) {
        goto done;
    }
    r->index = 0;
    r->status = ITER_READY;
    r->_data = target;
    r->next = array_iter_next;
done:
    return r;
}
