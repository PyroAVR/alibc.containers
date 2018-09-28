#pragma once
#include "debug.h"
#include "subscriber.h"

typedef struct dispatcher_S proto_dispatcher;
struct dispatcher_S     {
    int (*create_channel)(proto_dispatcher *self);
    void (*add_subscriber)(proto_dispatcher *self,
                           proto_subscriber *who, int which);
    void (*push_message)(proto_dispatcher* self, void *message, int to);
    void *impl;
};
