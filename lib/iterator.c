#include <alibc/containers/iterator.h>
#include <stddef.h>
/*
 * Internal status-check function
 */
static int check_status(iter_context *ctx);

void **iter_next(iter_context *ctx) {
    void **next_val = NULL;
    int status = check_status(ctx);
    if(status != ALC_ITER_READY && status != ALC_ITER_CONTINUE) {
        goto done;
    }
    next_val = ctx->next(ctx);
done:
    return next_val;
}

int iter_status(iter_context *ctx) {
    if(ctx == NULL) {
        return ALC_ITER_NULL;
    }
    return ctx->status;
}

void iter_free(iter_context *ctx) {
    if(ctx != NULL) {
        free(ctx);
    }
}

/*
 * Helper functions
 */
static int check_status(iter_context *ctx) {
    if(ctx == NULL) {
        return ALC_ITER_NULL;
    }
    if(ctx->next == NULL) {
        return ALC_ITER_INVALID;
    }
    return ALC_ITER_READY;
}
