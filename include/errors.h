#pragma once
typedef enum {
    SUCCESS,
    NO_MEM,
    NULL_ARG,
    NULL_IMPL,
    NULL_BUF,
    ARG_INVAL,
    IDX_OOB,
    IDX_NEG,
    STATE_INVAL
} alibc_internal_errors;
