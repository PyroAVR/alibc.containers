#include <alibc/extensions/iterator.h>
#include <stddef.h>
/*
 * Internal status-check function
 */
int check_status(iter_context *ctx);

void **iter_next(iter_context *ctx) {
    void **next_val = NULL;
    int status = check_status(ctx);
    if(status != ITER_READY && status != ITER_CONTINUE) {
        goto done;
    }
    next_val = ctx->next(ctx);
done:
    return next_val;
}

int iter_okay(iter_context *ctx) {
    if(ctx == NULL) {
        return ITER_NULL;
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
int check_status(iter_context *ctx) {
    if(ctx == NULL) {
        return ITER_NULL;
    }
    if(ctx->next == NULL) {
        return ITER_INVALID;
    }
    return ITER_READY;
}
