#include "../include/debug.h"
#include "../include/dynabuf.h"
#include <stdlib.h>
#include <string.h>

static dynabuf_status check_valid(dynabuf_t*);


// SIZE IN BYTES
dynabuf_t *create_dynabuf(int size) {
    dynabuf_t *r       = malloc(sizeof(dynabuf_t));
    if(r == NULL)   {
        DBG_LOG("Could not malloc dynabuf\n");
        return NULL;
    }
    r->buf  = malloc(size);
    if(r->buf == NULL)    {
        DBG_LOG("Could not malloc array\n");
        free(r);
        return NULL;
    }

    memset(r->buf, 0, size);
    r->capacity     = size;
    return r;
}


// SIZE IN BYTES
dynabuf_status dynabuf_resize(dynabuf_t *target, int size) {
    int status;
    void **newbuf = NULL;
    switch((status = check_valid(target)))   {
        case SUCCESS:
            newbuf  = realloc(target->buf, size);
            if(newbuf == NULL)  {
                DBG_LOG("Could not realloc buffer\n");
                return NO_MEM;
            }
            if(newbuf != target->buf)    {
                DBG_LOG("realloc moved target->buf, was: %p, is:%p\n",
                        target->buf, newbuf);
                target->buf = newbuf;
            }
            target->capacity = size;
            return status;

        break;
        default:
            DBG_LOG("Target was not valid: %d", status);
            return status;
    }
}

void dynabuf_free(dynabuf_t *target)    {
    if(target == NULL)  {
        return;
    }   else if(target->buf == NULL) {
        free(target);
    }   else    {
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
