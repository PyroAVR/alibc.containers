#include <alibc/extensions/debug.h>
#include <alibc/extensions/dynabuf.h>
#include <stdlib.h>
#include <string.h>

static dynabuf_status check_valid(dynabuf_t*);


// SIZE IN BYTES
dynabuf_t *create_dynabuf(int size, int unit) {
    dynabuf_t *r       = malloc(sizeof(dynabuf_t));
    if(r == NULL)   {
        DBG_LOG("Could not malloc dynabuf\n");
        return NULL;
    }
    r->buf  = malloc(size*unit);
    if(r->buf == NULL)    {
        DBG_LOG("Could not malloc array\n");
        free(r);
        return NULL;
    }

    memset(r->buf, 0, size*unit);
    r->capacity     = size*unit; // capacity is always in bytes for dynabuf.
    r->elem_size = unit;
    return r;
}


// SIZE IN ELEMENTS
dynabuf_status dynabuf_resize(dynabuf_t *target, int size) {
    int status = SUCCESS;
    char *newbuf = NULL;
    switch((status = check_valid(target)))   {
        case SUCCESS:
            newbuf  = realloc(target->buf, size*target->elem_size);
            if(newbuf == NULL)  {
                DBG_LOG("Could not realloc buffer\n");
                status = NO_MEM;
                goto done;
            }
            if(newbuf != target->buf)    {
                DBG_LOG("realloc moved target->buf, was: %p, is:%p\n",
                        target->buf, newbuf);
                target->buf = newbuf;
            }
            target->capacity = size*target->elem_size;
            goto done;

        break;
        default:
            DBG_LOG("Target was not valid: %d", status);
            goto done;
        break;
    }
done:
    return status;
}


int dynabuf_set(dynabuf_t *target, int which, void *element) {
    int status = SUCCESS;
    if((status = check_valid(target)) != SUCCESS) {
        goto done;
    }
    if(target->elem_size > sizeof(void*)) {
        memcpy(
            target->buf + (which * target->elem_size),
            element,
            target->elem_size
        );
    }
    else {
        memcpy(
            target->buf + (which * target->elem_size),
            &element,
            target->elem_size
        );
    }
done:
    return status;
}


int dynabuf_set_seq(dynabuf_t *target, int which, int next,
        void *value, int size) {
    int next_idx = 0;
    if(check_valid(target) != SUCCESS) {
        goto done;
    }
    if(next + size > target->elem_size) {
        next_idx = -1;
        goto done;
    }
    if(size > sizeof(void*)) {
        memcpy(
            target->buf + (which * target->elem_size) + next,
            value,
            size
        );
    }
    else {
        memcpy(
            target->buf + (which * target->elem_size) + next,
            &value,
            size
        );
    }
    next_idx += size;
    next_idx = next_idx >= target->elem_size ? 0: next_idx;
done:
    return next_idx;
}


void **dynabuf_fetch(dynabuf_t *target, int which) {
    void **r = NULL;
    if(check_valid(target) != SUCCESS) {
        goto done;
    }
    r = target->buf + (which * target->elem_size);
done:
    return r;
}


void dynabuf_free(dynabuf_t *target)    {
    if(target == NULL)  {
        return;
    }
    else if(target->buf == NULL) {
        free(target);
    }
    else    {
        free(target->buf);
        free(target);
    }
}


static dynabuf_status check_valid(dynabuf_t *self)  {
    if(self == NULL)    {
        return ARG_INVAL;
    }
    if(self->buf == NULL)   {
        return NULL_BUF;
    }
    if(self->capacity == 0)  {
        return NO_MEM;
    }
    return SUCCESS;
}
