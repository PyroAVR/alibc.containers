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
/*static array_status attempt_repair(array_t*);*/
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

    // zeroing-out the array allows swaps to swap-in NULL values, which is 
    // cleaner for the end-user.
    memset(r->data->buf, 0, size);
    r->size     = 0;
    r->status   = SUCCESS;

    return r;
}


array_status array_insert(array_t *self, int where, void *item)   {
    switch(check_space_available(self, 1))  {
        case SUCCESS:
            if(where > self->size)  {
                DBG_LOG("Attempted insert beyond end of array.\n");
                return IDX_OOB;
            }
            // decay to append operation if no swap is needed
            if(where != self->size) {
                // swapping allows for constant-time insertion, but 
                // changes the order of unrelated objects.
                array_swap(self, where, self->size);
            }
            self->data->buf[where]   = item;
            self->size++;
            self->status = SUCCESS;
        break;

        case NO_MEM:
            // try to allocate more space to store this item
            if(array_reallocate(self) == SUCCESS)   {
                DBG_LOG("realloc successful pre-insertion\n");
                return array_insert(self, where, item);
            }
            DBG_LOG("No space to insert new item. Ignoring.\n");
            self->status = NO_MEM;
            return NO_MEM;
        break;
        
        default:
            DBG_LOG("Array state was invalid on insert operation\n");
            self->status = STATE_INVAL;
        break;
    }
    self->status = SUCCESS;
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
                self->status = NO_MEM;
                return NO_MEM;
            }
        break;
        
        default:
            DBG_LOG("Array state was invalid on append operation\n");
            self->status = STATE_INVAL;
        break;
    }
    self->status = SUCCESS;
    return SUCCESS;
}


void *array_fetch(array_t *self, int which)    {
    switch(check_valid(self))   {
        case SUCCESS:
            if(which > self->size) {
                DBG_LOG("Requested fetch index was out of bounds: %d\n", which);
                self->status = IDX_OOB;
                return NULL;
            }
            else  {
                self->status = SUCCESS;
                return self->data->buf[which];
            }
        break;

        default:
            DBG_LOG("Array state was invalid on fetch operation\n");
            self->status = STATE_INVAL;
            return NULL;
    }
}


void *array_remove(array_t *self, int which)  {
    switch(check_valid(self))   {
        case SUCCESS:
            if(which >= self->size)  {
                DBG_LOG("Requested remove index was out of bounds: %d\n",
                        which);
                self->status = IDX_OOB;
                return NULL;
            }
            else    {
                array_swap(self, which, self->size);
                // the item specified by 'which' is now at the end of the array,
                // in the space after size
                self->status = SUCCESS;
                return self->data->buf[self->size--];
            }
        break;
        default:
            DBG_LOG("Array state was invalid on remove operation\n");
            self->status = STATE_INVAL;
            return NULL;
        break;

    }
}

array_status array_swap(array_t *self, int first, int second)    {
    int status;
    switch((status = check_valid(self)))   {
        case SUCCESS:
            // triple-xor swap
            self->data->buf[first]   = 
                (void*)((uint64_t)self->data->buf[first]
                ^ (uint64_t)self->data->buf[second]);
            self->data->buf[second]  =
                (void*)((uint64_t)self->data->buf[first]
                ^ (uint64_t)self->data->buf[second]);
            self->data->buf[first]   =
                (void*)((uint64_t)self->data->buf[first]
                ^ (uint64_t)self->data->buf[second]);
            // fall through here
        default:
            return status;
        break;
    }

}

int array_size(array_t *self) {
    switch(check_valid(self))   {
        case SUCCESS:
            self->status = SUCCESS;
            return self->size;
        break;

        default:
            self->status = STATE_INVAL;
            return -1;
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

/*
 * FIXME should this even be included here?
 * serious memory errors cannot be repaired automatically without system
 * intervention, and 'repairs' happening automatically could have adverse 
 * effect on debugging.
 */
/*
 *static array_status attempt_repair(array_t *self)    {
 *    switch(check_valid(self))   {
 *        case SUCCESS: return SUCCESS;
 *        case STATE_INVAL:
 *        break;
 *        case NULL_ARG:
 *            DBG_LOG("self was null\n");
 *            return NULL_ARG;
 *        break;
 *        case NULL_BUF:
 *            self->data          = NULL; // insurance
 *            self->data          = create_dynabuf(REPAIR_SIZE);
 *            if(self->data == NULL)  {
 *                DBG_LOG("Could not create dynabuf with new size %d\n",
 *                        REPAIR_SIZE);
 *                return STATE_INVAL;
 *            }
 *            self->size          = 0;
 *            return SUCCESS;
 *        break;
 *
 *        default:
 *            return STATE_INVAL;
 *    }
 *    return STATE_INVAL;
 *}
 */
// size in elements, not bytes
static array_status check_space_available(array_t *self, int elements)  {
    switch(check_valid(self))   {
        case SUCCESS:
            if(self->size + elements <= self->data->capacity/sizeof(void*)) {
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
    switch((status = check_valid(self)))   {
        case SUCCESS:
            switch((status = dynabuf_resize(self->data,
                            sizeof(void*)*(2*self->size + 1)))) {
                case SUCCESS:
                break;

                default:
                    DBG_LOG("Could not realloc dynabuf\n");
                break;
            }
            memset(self->data->buf + self->data->capacity, 0,
                    (self->data->capacity * 2 + 1) * sizeof(void*));
        break;
        
        default:
            DBG_LOG("Array state was invalid on resize operation\n");
            /*
             *if((status = attempt_repair(self)) == SUCCESS) {
             *    return array_reallocate(self);
             *}
             *else    {
             *    DBG_LOG("Could not repair array selfementation: %d\n", status);
             *    return status;
             *}
             */
        break;
    }
    return status;
}

//todo: 
//partition algs?
//errors?
