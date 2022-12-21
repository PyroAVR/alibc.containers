#pragma once
#ifdef __ALIBC_ENABLE_DEBUG_MACROS__
#include <stdio.h>

#undef NDEBUG
#include <assert.h>

#define DBG_LOG(x, ...) {\
    fprintf(stderr, "%s:%d:"x, \
    __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__);\
}

#define DBG_ASSERT(condition) assert(condition)

#else
#define DBG_LOG(x, ...)
#define DBG_ASSERT(condition)
#endif

