#pragma once
#include <stdint.h>
/**
 * Function prototype for alibc.containers comparator functions.
 * Any functions with this signature are guaranteed to work with comparator-
 * based containers defined in alibc.containers.
 */
typedef int8_t (cmp_type)(void *, void*);
