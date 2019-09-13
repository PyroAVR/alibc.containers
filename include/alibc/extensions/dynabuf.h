#pragma once
#include "errors.h"

typedef struct {
    void **buf;
    int capacity;
} dynabuf_t;

typedef alibc_internal_errors dynabuf_status;

dynabuf_t *create_dynabuf(int size);
dynabuf_status dynabuf_resize(dynabuf_t *target, int size);
void dynabuf_free(dynabuf_t *target);

