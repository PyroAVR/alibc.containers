#pragma once
#include <stdint.h>

/*
 * Comparison functions provided as sensible defaults.
 * These are meant to be for convenience, and are not chosen to be efficient nor
 * are they chosen to be robust.
 */
int8_t alc_default_cmp_i8(void *a, void *b);
int8_t alc_default_cmp_i16(void *a, void *b);
int8_t alc_default_cmp_i32(void *a, void *b);
int8_t alc_default_cmp_i64(void *a, void *b);
int8_t alc_default_cmp_ptr(void *a, void *b);
int8_t alc_default_cmp_str(void *a, void *b);

int8_t alc_default_cmp_u8(void *a, void *b);
int8_t alc_default_cmp_u16(void *a, void *b);
int8_t alc_default_cmp_u32(void *a, void *b);
int8_t alc_default_cmp_u64(void *a, void *b);
