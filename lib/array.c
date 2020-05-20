#include <alibc/extensions/array.h>
#include <alibc/extensions/dynabuf.h>
#include <alibc/extensions/errors.h>
#include <alibc/extensions/debug.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>


#define REPAIR_SIZE 8


// idea: "hardened" implementations which have data structure checksums

// private functions
static array_status check_valid(array_t*);
static array_status check_space_available(array_t*, int);

array_t *create_array(uint32_t size, uint32_t unit) {
    array_t *r       = malloc(sizeof(array_t));
    if(r == NULL)   {
        DBG_LOG("Could not malloc array_t\n");
        return NULL;
    }
    r->data          = create_dynabuf(size, unit);
    if(r->data == NULL)  {
        DBG_LOG("Could not create dynabuf with size %d\n", size);
        free(r);
        return NULL;
    }

    // zeroing-out the array allows swaps to swap-in NULL values, which is 
    // cleaner for the end-user.
    memset(r->data->buf, 0, size*unit);
    r->size     = 0;
    r->status   = SUCCESS;

    return r;
}


array_status array_insert(array_t *self, int where, void *item) {
    int status = SUCCESS;
    switch((status = check_space_available(self, 1))) {
        case SUCCESS:
            if(where > self->size) {
                DBG_LOG("Attempted insert beyond end of array.\n");
                status = IDX_OOB;
                goto done;
            }
            // decay to append operation if no swap is needed
            if(where <= self->size) {
                // swapping allows for constant-time insertion, but 
                // changes the order of unrelated objects.
                // place the object just past the end of the array
                array_swap(self, where, self->size);
            }
            status = dynabuf_set(self->data, where, item);
            if(status == SUCCESS) {
                self->size++;
            }
            goto done;
        break;

        case NO_MEM:
            // try to allocate more space to store this item
            if(array_resize(self, 2*self->size + 1) == SUCCESS) {
                DBG_LOG("realloc successful pre-insertion\n");
                status = array_insert(self, where, item);
                goto done;
            }
            DBG_LOG("No space to insert new item. Ignoring.\n");
            goto done;
        break;
        
        default:
            DBG_LOG("Array state was invalid on insert operation\n");
            goto invalid_self;
        break;
    }
done:
    self->status = status;
invalid_self:
    return status;
}


array_status array_insert_unsafe(array_t *self, int where, void *item) {
    array_status status;
    switch((status = check_valid(self))) {
        case SUCCESS:
        break;

        case NULL_ARG:
            DBG_LOG("self was not valid on unsafe insert.\n");
            goto invalid_self;
        break;

        default:
        DBG_LOG("Array was not valid on unsafe insert.\n");
        goto done;
        break;
    }

    if(where > self->size) {
        DBG_LOG("Attempted insert beyond end of array.\n");
        status = IDX_OOB;
        goto done;
    }
    // decay to append operation if no swap is needed
    /*
     *if(where != self->size) {
     *    // swapping allows for constant-time insertion, but 
     *    // changes the order of unrelated objects.
     *    array_swap(self, where, self->size);
     *}
     */
    dynabuf_set(self->data, where, item);
    status = SUCCESS;
    
done:
    self->status = status;
invalid_self:
    return status;
}


array_status array_append(array_t *self, void *item) {
    int status;
    switch((status = check_space_available(self, 1))) {
        case SUCCESS:
            status = dynabuf_set(self->data, self->size++, item);
        break;

        case NO_MEM:
            DBG_LOG("realloc needed\n");
            if((status = array_resize(self, 2*self->size + 1)) == SUCCESS) {
                status = array_append(self, item);
            }
            goto done;
        break;

        case NULL_ARG:
            goto invalid_self;
        break;

        default:
            DBG_LOG("Array state was invalid on append operation\n");
            goto done;
        break;
    }
done:
    self->status = status;
invalid_self:
    return status;
}


void **array_fetch(array_t *self, int which)    {
    void **r = NULL;
    if(check_valid(self) != SUCCESS) {
        DBG_LOG("Array state was invalid on fetch operation\n");
        goto done;
    }
    if(self->size == 0) {
        DBG_LOG("Requested fetch with size zero\n");
        self->status = IDX_OOB;
        goto done;
    }
    if(which >= self->size) {
        DBG_LOG("Requested fetch index was out of bounds: %d\n", which);
        self->status = IDX_OOB;
        goto done;
    }
    else  {
        self->status = SUCCESS;
        r = dynabuf_fetch(self->data, which);
    }
done:
    return r;
}


int array_resize(array_t *self, int count) {
    int status = SUCCESS;
    if(check_valid(self) != SUCCESS) {
        status = NULL_ARG;
        goto invalid_finish;
    }
    // no-op case
    if(count == self->size) {
        goto finish;
    }
    if(count*self->data->elem_size > self->data->capacity) {
        status = dynabuf_resize(self->data, count);
        if(status != SUCCESS) {
            DBG_LOG("Could not resize array backing buffer.\n");
            status = NO_MEM;
            goto finish;
        }
    }
/*
 *    else if(count >= self->size) {
 *        //resize and shrink size
 *        dynabuf_t *new_buf = create_dynabuf(count, self->data->elem_size);
 *        if(new_buf == NULL) {
 *            DBG_LOG("Could not create new (smaller) dynabuf for array.\n");
 *            status = NO_MEM;
 *            goto finish;
 *        }
 *        memcpy(new_buf->buf, self->data->buf, new_buf->elem_size*self->size);
 *        dynabuf_free(self->data);
 *        self->data = new_buf;
 *        status = SUCCESS;
 *        goto finish;
 *
 *    }
 */
    else {
        DBG_LOG("Requested array count %d was too small\n", count);
        status = IDX_OOB;
        goto finish;
    }

finish:
    self->status = status;
invalid_finish:
    return status;
}


void **array_remove(array_t *self, int which)  {
    void **r = NULL;
    int status;
    switch((status = check_valid(self))) {
        case SUCCESS:
        break;

        case NULL_ARG:
            DBG_LOG("self was invlaid on remove operation.\n");
            goto invalid_self;
        break;

        default:
            DBG_LOG("Array state was invalid on remove operation\n");
            goto done;
        break;
    }
    if(self->size == 0) {
        status = IDX_OOB;
        goto done;
    }
    if(which >= self->size)  {
        DBG_LOG("Requested remove index was out of bounds: %d\n",
                which);
        status = IDX_OOB;
        goto done;
    }
    else {
        // swap in the last valid
        array_swap(self, which, self->size-1);
        // the item specified by 'which' is now at the end of the array
        r = dynabuf_fetch(self->data, self->size-1);
        status = SUCCESS;
        self->size--;
    }
done:
    self->status = status;
invalid_self:
    return r;
}

array_status array_swap(array_t *self, int first, int second) {
    int status;
    if((status = check_valid(self)) != SUCCESS) {
        goto done;
    }
    // > instead of >= to allow swapping in a NULL without append...
    if(first > self->size || second > self->size) {
        status = IDX_OOB;
        goto done;
    }
    // due to larger-than-register sizes, we cannot simply swap values,
    // a true memory-to-memory copy may be needed (rats)
    if(self->data->elem_size <= sizeof(void*)) {
        void *tmp_first = *dynabuf_fetch(self->data, first);
        void *tmp_second = *dynabuf_fetch(self->data, second);
        dynabuf_set(self->data, first, tmp_second);
        dynabuf_set(self->data, second, tmp_first);
        status = SUCCESS;
    }
    else {
        char *cpy_buf = malloc(self->data->elem_size);
        if(cpy_buf == NULL) {
            status = NO_MEM;
            goto done;
        }
        memcpy(
            cpy_buf, dynabuf_fetch(self->data, first), self->data->elem_size
        );
        dynabuf_set(self->data, first, dynabuf_fetch(self->data, second));
        dynabuf_set(self->data, second, cpy_buf);
        free(cpy_buf);
        status = SUCCESS;
    }
done:
    return status;
}

int array_size(array_t *self) {
    switch(check_valid(self))   {
        case SUCCESS:
            self->status = SUCCESS;
            return self->size;
        break;

        case NULL_ARG:
            return -1;
        break;

        default:
            return -1;
        break;
    }
}

void array_free(array_t *self)    {
    if(self == NULL)    {
        DBG_LOG("self was null\n");
        return;
    }
    if(self->data == NULL)  {
        DBG_LOG("dynabuf was null\n");
        free(self);
    }
    else {
        dynabuf_free(self->data);
        free(self);
        return;
    }
}

inline int array_okay(array_t *self)    {
    return self->status;
}
/*
 *Helper functions
 */

// possible returns: SUCCESS NULL_ARG NULL_IMPL NULL_BUF
static array_status check_valid(array_t *self)    {
    if(self == NULL)    {
        return NULL_ARG;
    }
    
    if(self->data == NULL) {
        return NULL_BUF;
    }
    return SUCCESS;
}

// size in elements, not bytes
static array_status check_space_available(array_t *self, int elements)  {
    int status = SUCCESS;
    switch((status = check_valid(self))) {
        case SUCCESS:
            if(self->size + elements >
               self->data->capacity/self->data->elem_size) {
                status = NO_MEM;
            }
        break;
    }
    return status;
} 

//todo: 
//partition algs?
//errors?
