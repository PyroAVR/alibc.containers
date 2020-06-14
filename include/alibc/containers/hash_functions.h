#pragma once
#include <stdint.h>

/*
 * Hash functions provided as sensible defaults.
 * These are meant to be for convenience, and are not chosen to be efficient nor
 * are they chosen to be robust.
 */
uint32_t alc_default_hash_i8(void *val);
uint32_t alc_default_hash_i16(void *val);
uint32_t alc_default_hash_i32(void *val);
uint32_t alc_default_hash_i64(void *val);
uint32_t alc_default_hash_ptr(void *ptr);
uint32_t alc_default_hash_str(void *str);
