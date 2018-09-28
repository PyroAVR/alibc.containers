#include "../include/list.h"
#include "../include/array.h"
#include "../include/debug.h"
#include <stdlib.h>
#include <stdint.h>


#define REPAIR_SIZE 8


// idea: "hardened" implementations which have data structure checksums

// public functions

static void array_insert(proto_list*, int, void*);
static void array_append(proto_list*, void*);
static void *array_fetch(proto_list*, int);
static void *array_remove(proto_list*, int);
static void array_swap(proto_list*, int, int);
static int array_size(proto_list*);
static void array_free(proto_list*);



// private state
#define impl_cast(x) ((array_impl_t*)(x))
typedef struct  {
    void **data;
    uint32_t size;
    uint32_t capacity;
    uint8_t status;
} array_impl_t;

typedef enum    {
    SUCCESS,
    NO_MEM,
    NULL_ARG,
    NULL_IMPL,
    NULL_BUF,
    IDX_OOB,
    IDX_NEG,
    STATE_INVAL
} array_status;

// private functions
static array_status check_valid(proto_list*);
static array_status check_space_available(proto_list*, int);
static array_status attempt_repair(proto_list*);
static array_status array_reallocate(proto_list*);

proto_list *create_array(uint32_t size) {
    proto_list *r       = malloc(sizeof(proto_list));
    if(r == NULL)   {
        DBG_LOG("Could not malloc proto_list\n");
        return NULL;
    }
    array_impl_t *impl  = malloc(sizeof(array_impl_t));
    if(impl == NULL)    {
        DBG_LOG("Could not malloc array implementation\n");
        free(r);
        return NULL;
    }
    impl->data          = NULL; // insurance
    impl->data          = malloc(sizeof(void*)*size);
    if(impl->data == NULL)  {
        DBG_LOG("Could not malloc raw buffer of size %s\n", size);
        free(impl);
        free(r);
        return NULL;
    }
    impl->size          = 0;
    impl->capacity      = size;

    r->impl             = impl;
    r->insert           = array_insert;
    r->append           = array_append;
    r->fetch            = array_fetch;
    r->remove           = array_remove;
    r->swap             = array_swap;
    r->size             = array_size;
    r->free             = array_free;
    return r;

}


static void array_insert(proto_list *self, int where, void *item)   {
    array_impl_t *impl;
    switch(check_space_available(self, 1))  {
        case SUCCESS:
            impl    = impl_cast(self->impl);
            array_swap(self, where, impl->size);
            impl->data[where]   = item;
            impl->size++;
    }
}


static void array_append(proto_list *self, void *item)  {
    int status;
    switch(check_space_available(self, 1))   {
        case SUCCESS:
            impl_cast(self->impl)->data[impl_cast(self->impl)->size++] = item;
        break;
        case NO_MEM:
            DBG_LOG("realloc needed\n");
            if((status = array_reallocate(self)) == SUCCESS)    {
                // error codes pls
                array_append(self, item);
                return;
            }   else    {
                // return status;
            }
        break;
        default:
            switch(attempt_repair(self))    {
                case SUCCESS:
                    array_append(self, item);
                break;
                default: //FIXME silent failure is not okay
                break;
            }
    }

}


static void *array_fetch(proto_list *self, int which)    {
    switch(check_valid(self))   {
        case SUCCESS:
            if(which > impl_cast(self->impl)->size) {
                return NULL;
            } else  {
                return impl_cast(self->impl)->data[which];
            }
        break;
        default:
            return NULL;
    }
}


static void *array_remove(proto_list *self, int which)  {
    array_impl_t *impl;
    switch(check_valid(self))   {
        case SUCCESS:
            impl    = impl_cast(self->impl);
            if(which > impl->size)  {
                return NULL;
            }   else    {
                array_swap(self, which, impl->size -1);
                impl->size--;
                return impl->data[impl->size -1];
            }
        break;
        default:
            return NULL;
        break;

    }
}

static void array_swap(proto_list *self, int first, int second)    {
    array_impl_t *impl;
    switch(check_valid(self))   {
        case SUCCESS:
            impl    = impl_cast(self->impl);
            impl->data[first]   = 
            (void*)((uint64_t)impl->data[first] ^ (uint64_t)impl->data[second]);
            impl->data[second]  =
            (void*)((uint64_t)impl->data[first] ^ (uint64_t)impl->data[second]);
            impl->data[first]   =
            (void*)((uint64_t)impl->data[first] ^ (uint64_t)impl->data[second]);
        default:
            // aaaaaa
        break;
    }

}

static int array_size(proto_list *self) {
    if(check_valid(self) != SUCCESS)   {
        if(attempt_repair(self) != SUCCESS) {
            return -1; // FIXME: return a status code! 
        }
    }
    return impl_cast(self->impl)->size;
}

static void array_free(proto_list *self)    {
    if(self == NULL)    {
        DBG_LOG("self was null");
        return;
    }
    if(self->impl == NULL)  {
        DBG_LOG("self->impl was null");
        free(self);
        return;
    }
    if(impl_cast(self->impl)->data != NULL) {
        free(impl_cast(self->impl)->data);
        free(self->impl);
        free(self);
        return;
    }
}


/*
 *Helper functions
 */

// possible returns: SUCCESS NULL_ARG NULL_IMPL NULL_BUF
static array_status check_valid(proto_list *self)    {
    if(self == NULL)    {
        return NULL_ARG;
    }
    
    if(self->impl == NULL)  {
        return NULL_IMPL;
    }

    if(impl_cast(self->impl)->data == NULL) {
        return NULL_BUF;
    }
    return SUCCESS;
}


static array_status attempt_repair(proto_list *self)    {
    array_impl_t *impl          = NULL;
    switch(check_valid(self))   {
        case SUCCESS: return SUCCESS;
        case STATE_INVAL:
        break;
        case NULL_ARG:
            return NULL_ARG;
        break;
        case NULL_IMPL:
            DBG_LOG("lost ref to impl cleanly (null)... memory leak likely.");
            impl                = malloc(sizeof(array_impl_t));
            if(impl == NULL)    {
                DBG_LOG("Could not malloc new impl");
                return STATE_INVAL;
            }

        case NULL_BUF:  // FALLTHROUGH INTENTIONAL
            impl                = impl_cast(self->impl);
            impl->data          = NULL; // insurance
            impl->data          = malloc(sizeof(void*)*REPAIR_SIZE);
            if(impl->data == NULL)  {
                DBG_LOG("Could not malloc raw buffer");
                return STATE_INVAL;
            }
            impl->size          = 0;
            impl->capacity      = REPAIR_SIZE;
            self->impl          = impl;
            return SUCCESS;
        break;

        default:
            return STATE_INVAL;
    }
    return STATE_INVAL;
}
// size in elements, not bytes
static array_status check_space_available(proto_list *self, int elements)  {
    array_impl_t *impl;
    switch(check_valid(self))   {
        case SUCCESS:
            impl = impl_cast(self->impl);
            if(impl->size + elements <= impl->capacity) {
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

static array_status array_reallocate(proto_list *self)  {
    int status;
    void **newbuf;
    array_impl_t *impl;
    switch(check_valid(self))   {
        case SUCCESS:
            impl    = impl_cast(self->impl);
            // default scaling policy 2n + 1
            newbuf  = realloc(impl->data, sizeof(void*)*(impl->size * 2 + 1));
            if(newbuf == NULL)  {
                DBG_LOG("Could not realloc buffer\n");
                return NO_MEM;
            }
            if(newbuf != impl->data)    {
                DBG_LOG("realloc moved impl->data, was: %p, is:%p\n",
                        impl->data, newbuf);
                impl->data = newbuf;
            }
            impl->capacity  = impl->capacity * 2 + 1;
        break;
        
        default:
            if((status = attempt_repair(self)) == SUCCESS) {
                return array_reallocate(self);
            }   else    {
                return status;
            }
        break;
    }
    return SUCCESS;
}

//todo: 
//partition algs?
//errors?
