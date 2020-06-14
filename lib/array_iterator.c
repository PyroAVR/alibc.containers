#include <alibc/containers/array_iterator.h>
#include <alibc/containers/array.h>
#include <stdlib.h>

static void *array_iter_next(iter_context *ctx) {
    void *r = NULL;
    array_t *target = (array_t*)ctx->_data;
    if(target == NULL) {
        ctx->status = ALC_ITER_INVALID;
        goto done;
    }
    if(ctx->index < array_size(target)) {
        r = array_fetch(target, ctx->index);
        ctx->index++;
    }
    if(ctx->index >= array_size(target) || array_size(target) == 0) {
        ctx->status = ALC_ITER_STOP;
    }
    else {
        ctx->status = ALC_ITER_CONTINUE;
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
    r->status = ALC_ITER_READY;
    r->_data = target;
    r->next = array_iter_next;
done:
    return r;
}
