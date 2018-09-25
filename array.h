#pragma once
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>

typedef struct  {
    void **data;
    uint32_t size;
    uint32_t capacity;
    uint8_t status;
} array_impl_t;

#pragma pack()
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

proto_list *create_array(uint32_t size);
