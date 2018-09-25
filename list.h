#pragma once
#ifdef DEBUG
#include <stdio.h>
#define DBG_LOG(x, ...) {\
    fprintf(stderr, "%s:%d:"x, \
    __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__);\
}
#else
#define DBG_LOG(x, ...)
#endif

typedef struct list_S proto_list;
typedef struct list_S   {
    //void *(*create)(proto_list* self, int init_size);
    void (*insert)(proto_list *self, int where, void *item);
    void (*append)(proto_list *self, void *item);
    void *(*fetch)(proto_list *self, int which);
    void *(*remove)(proto_list *self, int which);
    void (*swap)(proto_list *self, int first, int second);
    int (*size)(proto_list *self);
    void (*free)(proto_list *self);
    void *impl;
} proto_list;
