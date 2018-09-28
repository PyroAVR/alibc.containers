#pragma once
#include "debug.h"
typedef struct subscriber_S proto_subscriber;
struct subscriber_S     {
    void (*notify)(void *message);
    void *self;
};

