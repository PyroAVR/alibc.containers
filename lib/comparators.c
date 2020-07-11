#include <string.h>
#include <stdint.h>

/*
 * The strange constants here are to enforce that only the valid and expected
 * bits of the arguments are considered during hashing and comparison.
 */

int8_t alc_default_cmp_i8(void *a, void *b){
    int8_t v = ((0xffULL)&(int64_t)a) - ((0xffULL)&(int64_t)b);
    return (v == 0) ? 0:(v > 0)? 1:-1;
}

int8_t alc_default_cmp_i16(void *a, void *b){
    int16_t v = ((0xffffULL)&(int64_t)a) - ((0xffffULL)&(int64_t)b);
    return (v == 0) ? 0:(v > 0)? 1:-1;
}

int8_t alc_default_cmp_i32(void *a, void *b){
    int32_t v = ((0xffffffffULL)&(int64_t)a)
                    - ((0xffffffffULL)&(int64_t)b);
    return (v == 0) ? 0:(v > 0)? 1:-1;
}

int8_t alc_default_cmp_i64(void *a, void *b){
    int64_t v = (int64_t)a - (int64_t)b;
    return (v == 0) ? 0:(v > 0)? 1:-1;
}

int8_t alc_default_cmp_ptr(void *a, void *b){
    return alc_default_cmp_i64(a, b);
}

int8_t alc_default_cmp_str(void *a, void *b){
    int idx = 0;
    int8_t status = 0;
    char *c = (char*)a;
    char *d = (char*)b;
    if(a == NULL || b == NULL)  {
        return 0;
    }
    while(c[idx] != 0 && d[idx] != 0)   {
        if(c[idx] > d[idx]) {
            status = 1;
            goto done;
        }
        else if(c[idx] < d[idx]) {
            status = -1;
            goto done;
        }
        idx++;
    }
done:
    return status;
}

int8_t alc_default_cmp_u8(void *a, void *b){
    int8_t v = ((0xffULL)&(uint64_t)a) - ((0xffULL)&(uint64_t)b);
    return (v == 0) ? 0:(v > 0)? 1:-1;
}

int8_t alc_default_cmp_u16(void *a, void *b){
    int16_t v = ((0xffffULL)&(uint64_t)a) - ((0xffffULL)&(uint64_t)b);
    return (v == 0) ? 0:(v > 0)? 1:-1;
}

int8_t alc_default_cmp_u32(void *a, void *b){
    int32_t v = ((0xffffffffULL)&(uint64_t)a)
                    - ((0xffffffffULL)&(uint64_t)b);
    return (v == 0) ? 0:(v > 0)? 1:-1;
}

int8_t alc_default_cmp_u64(void *a, void *b){
    int64_t v = (uint64_t)a - (uint64_t)b;
    return (v == 0) ? 0:(v > 0)? 1:-1;
}
