#pragma once
#include <stdint.h>
#include "dynabuf.h"

typedef struct  {
    dynabuf_t *data;
    uint32_t size;
    uint8_t status;
} array_t;

typedef alibc_internal_errors array_status;

array_t *create_array(uint32_t size);
array_status array_insert(array_t*, int, void*);
array_status array_append(array_t*, void*);
void *array_fetch(array_t*, int);
void *array_remove(array_t*, int);
array_status array_swap(array_t*, int, int);
int array_size(array_t*);
void array_free(array_t*);
