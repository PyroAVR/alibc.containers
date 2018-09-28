#pragma once
typedef struct list_S proto_list;
struct list_S   {
    //void *(*create)(proto_list* self, int init_size);
    void (*insert)(proto_list *self, int where, void *item);
    void (*append)(proto_list *self, void *item);
    void *(*fetch)(proto_list *self, int which);
    void *(*remove)(proto_list *self, int which);
    void (*swap)(proto_list *self, int first, int second);
    int (*size)(proto_list *self);
    void (*free)(proto_list *self);
    void *impl;
};
