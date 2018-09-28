#pragma once
#include <stdint.h>
typedef struct map_S proto_map;
struct map_S    {
    void (*set)(proto_map *self, void *key, void *value);
    void *(*fetch)(proto_map *self, void *key);
    void (*remove)(proto_map *self, void *key);
    uint32_t (*size)(proto_map *self);
    void (*free)(proto_map *self);
    void *impl;
};
