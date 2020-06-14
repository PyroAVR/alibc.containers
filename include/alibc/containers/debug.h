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

