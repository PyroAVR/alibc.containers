#pragma once
#include <stdint.h>
/**
 * Function prototype for alibc.containers hash functions.
 * Any functions with this signature are guaranteed to work with the hash-based
 * containers defined in alibc.containers.
 */
typedef uint32_t (hash_type)(void *);
