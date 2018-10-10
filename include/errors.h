#pragma once
#include <limits.h>
typedef enum {
    SUCCESS = 0,
    NO_MEM = INT_MIN, // u h h  h
    NULL_ARG,
    NULL_IMPL,
    NULL_BUF,
    ARG_INVAL,
    IDX_OOB,
    IDX_NEG,
    STATE_INVAL,
    BAD_PARAM,
    NULL_HASH
} alibc_internal_errors;
