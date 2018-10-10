#include "../include/array.h"
#include "../include/dynabuf.h"
#include "../include/errors.h"
#include "../include/debug.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>


#define REPAIR_SIZE 8


// idea: "hardened" implementations which have data structure checksums

// public functions




// private state


// private functions
static array_status check_valid(array_t*);
static array_status check_space_available(array_t*, int);
static array_status attempt_repair(array_t*);
static array_status array_reallocate(array_t*);

array_t *create_array(uint32_t size) {
    array_t *r       = malloc(sizeof(array_t));
    if(r == NULL)   {
        DBG_LOG("Could not malloc array_t\n");
        return NULL;
    }
    r->data          = create_dynabuf(size*sizeof(void*));
    if(r->data == NULL)  {
        DBG_LOG("Could not create dynabuf with size %d\n", size);
        free(r);
        return NULL;
    }

    memset(r->data->buf, 0, size);
    r->size          = 0;

    return r;
}


array_status array_insert(array_t *self, int where, void *item)   {
    switch(check_space_available(self, 1))  {
        case SUCCESS:
            if(where > self->size)  {
                DBG_LOG("Attempted insert beyond end of array.\n");
                return IDX_OOB;
            }
            if(where != self->size) {
                array_swap(self, where, self->size);
            }
            self->data->buf[where]   = item;
            self->size++;
        break;

        case NO_MEM:
            if(array_reallocate(self) == SUCCESS)   {
                DBG_LOG("realloc successful pre-insertion\n");
                return array_insert(self, where, item);
            }
            DBG_LOG("No space to insert new item. Ignoring.\n");
            return NO_MEM;
        break;
        
        default:
            DBG_LOG("No space was available and state was otherwise invalid\n");
        break;
    }
    return SUCCESS;
}


array_status array_append(array_t *self, void *item)  {
    int status;
    switch(check_space_available(self, 1))   {
        case SUCCESS:
            self->data->buf[self->size++] = item;
        break;
        case NO_MEM:
            DBG_LOG("realloc needed\n");
            if((status = array_reallocate(self)) == SUCCESS)    {
                return array_append(self, item);
            }
            else    {
                return NO_MEM;
            }
        break;
        default:
            switch((status = attempt_repair(self)))    {
                case SUCCESS:
                    return array_append(self, item);
                break;
                default: 
                return status;
                break;
            }
    }
    return SUCCESS;
}


void *array_fetch(array_t *self, int which)    {
    switch(check_valid(self))   {
        case SUCCESS:
            if(which > self->size) {
                DBG_LOG("Requested fetch index was out of bounds: %d\n", which);
                return NULL;
            }
            else  {
                return self->data->buf[which];
            }
        break;
        default:
            return NULL;
    }
}


void *array_remove(array_t *self, int which)  {
    switch(check_valid(self))   {
        case SUCCESS:
            if(which > self->size)  {
                DBG_LOG("Requested remove index was out of bounds: %d\n",
                        which);
                return NULL;
            }
            else    {
                array_swap(self, which, self->size -1);
                self->size--;
                return self->data->buf[self->size -1];
            }
        break;
        default:
            return NULL;
        break;

    }
}

array_status array_swap(array_t *self, int first, int second)    {
    int status;
    switch((status = check_valid(self)))   {
        case SUCCESS:
            self->data->buf[first]   = 
                (void*)((uint64_t)self->data->buf[first]
                ^ (uint64_t)self->data->buf[second]);
            self->data->buf[second]  =
                (void*)((uint64_t)self->data->buf[first]
                ^ (uint64_t)self->data->buf[second]);
            self->data->buf[first]   =
                (void*)((uint64_t)self->data->buf[first]
                ^ (uint64_t)self->data->buf[second]);
        default:
            return status;
        break;
    }

}

int array_size(array_t *self) {
    int status;
    if((status = check_valid(self)) != SUCCESS)   {
        if(attempt_repair(self) != SUCCESS) {
            return status;
        }
    }
    return self->size;
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


static array_status attempt_repair(array_t *self)    {
    switch(check_valid(self))   {
        case SUCCESS: return SUCCESS;
        case STATE_INVAL:
        break;
        case NULL_ARG:
            DBG_LOG("self was null\n");
            return NULL_ARG;
        break;
        case NULL_BUF:
            self->data          = NULL; // insurance
            self->data          = create_dynabuf(REPAIR_SIZE);
            if(self->data == NULL)  {
                DBG_LOG("Could not create dynabuf with new size %d\n",
                        REPAIR_SIZE);
                return STATE_INVAL;
            }
            self->size          = 0;
            return SUCCESS;
        break;

        default:
            return STATE_INVAL;
    }
    return STATE_INVAL;
}
// size in elements, not bytes
static array_status check_space_available(array_t *self, int elements)  {
    switch(check_valid(self))   {
        case SUCCESS:
            if(self->size + elements < self->data->capacity/sizeof(void*)) {
                return SUCCESS;
            }
            else    {
                return NO_MEM;
            }
        break;

        default:
            return NO_MEM;
    }
    return SUCCESS;
} 

array_status array_reallocate(array_t *self)  {
    int status;
    void **newbuf;
    switch(check_valid(self))   {
        case SUCCESS:
            switch((status = dynabuf_resize(self->data,
                            sizeof(void*)*(2*self->size + 1)))) {
                case SUCCESS:
                    return status;
                break;

                default:
                    DBG_LOG("Could not realloc dynabuf\n");
                    return status;
                break;
            }
            memset(self->data->buf + self->data->capacity, 0,
                    (self->data->capacity * 2 + 1) * sizeof(void*));
        break;
        
        default:
            if((status = attempt_repair(self)) == SUCCESS) {
                return array_reallocate(self);
            }
            else    {
                DBG_LOG("Could not repair array selfementation: %d\n", status);
                return status;
            }
        break;
    }
    return SUCCESS;
}

//todo: 
//partition algs?
//errors?
