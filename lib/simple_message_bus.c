#include "../include/simple_message_bus.h"


// public functions
static int create_channel(proto_dispatcher *self);
static void add_subscriber(proto_dispatcher *self,
                           proto_subscriber *who, int which);
static void push_message(proto_dispatcher *self, void *message, int to);

// private functions
static msgbus_status check_valid(proto_dispatcher *self);
// private state
#define impl_cast(x) ((msgbus_impl_t*)(x))
typedef struct {
    proto_list *channels;
    //TODO proto_hashtab *channels
} msgbus_impl_t;

typedef enum    {
    SUCCESS,
    NULL_ARG,
    NULL_IMPL,
    NULL_CHAN,
    CHAN_OOB
} msgbus_status;

proto_dispatcher *create_message_bus(void)  {
    proto_dispatcher *r = malloc(sizeof(proto_dispatcher));
    if(r == NULL)    {
        DBG_LOG("Could not malloc proto_dispatcher\n");
        return NULL;
    }

    r->impl     = malloc(sizeof(msgbus_impl_t));
    if(r->impl == NULL)  {
        DBG_LOG("Could not malloc msgbus_impl_t\n");
        free(r);
        return NULL;
    }

    impl_cast(r->impl)->channels    = create_array(1);
    if(impl_cast(r->impl)->channels == NULL)    {
        DBG_LOG("Could not create new array for subscribers\n");
        free(r->impl);
        free(r);
        return NULL;
    }

    r->create_channel   = create_channel;
    r->add_subscriber   = add_subscriber;
    r->push_message     = push_message;
    
    return r;
}

int create_channel(proto_dispatcher *self)  {
    msgbus_impl_t *impl;
    switch(check_valid(self))   {
        case SUCCESS:
            impl    = impl_cast(self->impl);
            /*impl->channels->*/
        break;

        default:

        break;
    }
}

void add_subscriber(proto_dispatcher *self, proto_subscriber *who, int which) {

}

void push_message(proto_dispatcher *self, void *message, int to)    {

}

/*
 * Helper functions
 */

// possible returns: SUCCESS NULL_ARG NULL_IMPL NULL_BUF
static msgbus_status check_valid(proto_dispatcher *self)    {
    if(self == NULL)    {
        return NULL_ARG;
    }
    
    if(self->impl == NULL)  {
        return NULL_IMPL;
    }

    if(impl_cast(self->impl)->channels == NULL) {
        return NULL_CHAN;
    }
    return SUCCESS;
}
