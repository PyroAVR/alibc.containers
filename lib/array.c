#include <alibc/containers/array.h>
#include <alibc/containers/dynabuf.h>
#include <alibc/containers/debug.h>
#include <stdlib.h>
#include <stdint.h>

// private functions
static int check_valid(array_t*);
static int check_space_available(array_t*, int);

array_t *create_array(int size, int unit) {
    array_t *r       = malloc(sizeof(array_t));
    if(r == NULL)   {
        DBG_LOG("Could not malloc array_t\n");
        goto done;
    }
    r->data          = create_dynabuf(size, unit);
    if(r->data == NULL)  {
        DBG_LOG("Could not create dynabuf with size %d\n", size);
        free(r);
        r = NULL;
        goto done;
    }

    r->size     = 0;
    r->status   = ALC_ARRAY_SUCCESS;

done:
    return r;
}


int array_insert(array_t *self, int where, void *item) {
    int status = check_space_available(self, 1);
    switch(status) {
        case ALC_ARRAY_SUCCESS:
            if(where > self->size) {
                DBG_LOG("Attempted insert beyond end of array.\n");
                status = ALC_ARRAY_IDX_OOB;
                goto done;
            }
            // decay to append operation if no swap is needed
            if(where <= self->size) {
                // swapping allows for constant-time insertion, but 
                // changes the order of unrelated objects.
                // place the object just past the end of the array
                array_swap(self, where, self->size);
            }
            dynabuf_set(self->data, where, item);
            self->size++;
            goto done;
        break;

        case ALC_ARRAY_NO_MEM:
            // try to allocate more space to store this item
            status = array_resize(self, 2*self->size + 1);
            if(status == ALC_ARRAY_SUCCESS) {
                DBG_LOG("realloc successful pre-insertion\n");
                status = array_insert(self, where, item);
                goto done;
            }
            DBG_LOG("No space to insert new item. Ignoring.\n");
            goto done;
        break;
        
        default:
            DBG_LOG("Array state was invalid on insert operation\n");
            goto invalid_status;
        break;
    }
done:
    self->status = status;
invalid_status:
    return status;
}


int array_insert_unsafe(array_t *self, int where, void *item) {
    int status = check_valid(self);
    if(status != ALC_ARRAY_SUCCESS) {
        DBG_LOG("Array was not valid on unsafe insert.\n");
        goto invalid_status;
    }

    if(where > self->size) {
        DBG_LOG("Attempted insert beyond end of array.\n");
        status = ALC_ARRAY_IDX_OOB;
        goto done;
    }
    dynabuf_set(self->data, where, item);
    
done:
    self->status = status;
invalid_status:
    return status;
}


int array_append(array_t *self, void *item) {
    int status;
    switch((status = check_space_available(self, 1))) {
        case ALC_ARRAY_SUCCESS:
            dynabuf_set(self->data, self->size++, item);
        break;

        case ALC_ARRAY_NO_MEM:
            DBG_LOG("realloc needed\n");
            status = array_resize(self, 2*self->size + 1);
            if(status == ALC_ARRAY_SUCCESS) {
                status = array_append(self, item);
            }
            goto done;
        break;

        default:
            DBG_LOG("Array state was invalid on append operation\n");
            goto invalid_status;
        break;
    }
done:
    self->status = status;
invalid_status:
    return status;
}


void **array_fetch(array_t *self, int which)    {
    void **r = NULL;
    int status = ALC_ARRAY_SUCCESS;
    if(check_valid(self) != ALC_ARRAY_SUCCESS) {
        DBG_LOG("Array state was invalid on fetch operation\n");
        goto invalid_status;
    }
    if(self->size == 0) {
        DBG_LOG("Requested fetch with size zero\n");
        status = ALC_ARRAY_IDX_OOB;
        goto done;
    }
    if(which >= self->size || which < 0) {
        DBG_LOG("Requested fetch index was out of bounds: %d\n", which);
        status = ALC_ARRAY_IDX_OOB;
        goto done;
    }
    else  {
        self->status = ALC_ARRAY_SUCCESS;
        r = dynabuf_fetch(self->data, which);
    }
done:
    self->status = status;
invalid_status:
    return r;
}


int array_resize(array_t *self, int count) {
    int status = check_valid(self);
    if(status != ALC_ARRAY_SUCCESS) {
        goto invalid_status;
    }
    // no-op case
    if(count == self->size) {
        goto done;
    }
    if(count*self->data->elem_size > self->data->capacity) {
        if(dynabuf_resize(self->data, count) != ALC_DYNABUF_SUCCESS) {
            DBG_LOG("Could not resize array backing buffer.\n");
            status = ALC_ARRAY_NO_MEM;
            goto done;
        }
    }
    else {
        DBG_LOG("Requested array count %d was too small\n", count);
        status = ALC_ARRAY_IDX_OOB;
        goto done;
    }

done:
    self->status = status;
invalid_status:
    return status;
}


void **array_remove(array_t *self, int which)  {
    void **r = NULL;
    int status = check_valid(self);
    if(status != ALC_ARRAY_SUCCESS) {
        DBG_LOG("self was invlaid on remove operation.\n");
        goto invalid_status;
    }

    if(self->size == 0) {
        status = ALC_ARRAY_IDX_OOB;
        goto done;
    }
    if(which >= self->size)  {
        DBG_LOG("Requested remove index was out of bounds: %d\n",
                which);
        status = ALC_ARRAY_IDX_OOB;
        goto done;
    }
    else {
        // swap in the last valid
        array_swap(self, which, self->size-1);
        // the item specified by 'which' is now at the end of the array
        r = dynabuf_fetch(self->data, self->size-1);
        self->size--;
    }
done:
    self->status = status;
invalid_status:
    return r;
}

int array_swap(array_t *self, int first, int second) {
    int status = check_valid(self);
    if(status != ALC_ARRAY_SUCCESS) {
        goto invalid_status;
    }
    // > instead of >= to allow swapping in a NULL without append...
    if(first > self->size || second > self->size) {
        status = ALC_ARRAY_IDX_OOB;
        goto done;
    }
    // due to larger-than-register sizes, we cannot simply swap values,
    // a true memory-to-memory copy may be needed (rats)
    if(self->data->elem_size <= sizeof(void*)) {
        void *tmp_first = *dynabuf_fetch(self->data, first);
        void *tmp_second = *dynabuf_fetch(self->data, second);
        dynabuf_set(self->data, first, tmp_second);
        dynabuf_set(self->data, second, tmp_first);
        status = ALC_ARRAY_SUCCESS;
    }
    else {
        char *cpy_buf = malloc(self->data->elem_size);
        if(cpy_buf == NULL) {
            status = ALC_ARRAY_NO_MEM;
            goto done;
        }
        memcpy(
            cpy_buf, dynabuf_fetch(self->data, first), self->data->elem_size
        );
        dynabuf_set(self->data, first, dynabuf_fetch(self->data, second));
        dynabuf_set(self->data, second, cpy_buf);
        free(cpy_buf);
    }

done:
    self->status = status;
invalid_status:
    return status;
}

int array_size(array_t *self) {
    int size = -1;
    if(check_valid(self) == ALC_ARRAY_SUCCESS)   {
        self->status = ALC_ARRAY_SUCCESS;
        size = self->size;
    }
    return size;
}

void array_free(array_t *self)    {
    if(self == NULL)    {
        DBG_LOG("self was null\n");
    }
    else if(self->data == NULL)  {
        DBG_LOG("dynabuf was null\n");
        free(self);
    }
    else {
        dynabuf_free(self->data);
        free(self);
    }
}

inline int array_status(array_t *self)    {
    return self->status;
}

/*
 *Helper functions
 */

static int check_valid(array_t *self)    {
    int status = ALC_ARRAY_SUCCESS;
    if(self == NULL)    {
        status = ALC_ARRAY_INVALID;
        goto done;
    }
    
    if(self->data == NULL) {
        return ALC_ARRAY_INVALID;
    }
done:
    return status;
}

// size in elements, not bytes
static int check_space_available(array_t *self, int elements)  {
    int status = check_valid(self);
    if(status == ALC_ARRAY_SUCCESS) {
        if(self->size + elements > self->data->capacity/self->data->elem_size) {
            status = ALC_ARRAY_NO_MEM;
        }
    }
done:
    return status;
} 
