#include <alibc/extensions/hashmap.h>
#include <alibc/extensions/debug.h>
#include <string.h>

/*
 * The strange constants here are to enforce that only the valid and expected
 * bits of the arguments are considered during hashing and comparison.
 */

inline int8_t hashmap_cmp_i8(void *a, void *b){
    uint8_t v   = ((0xffULL)&(uint64_t)a) - ((0xffULL)&(uint64_t)b);
    return (v == 0) ? 0:(v > 0)? 1:-1;
}

inline int8_t hashmap_cmp_i16(void *a, void *b){
    uint16_t v   = ((0xffffULL)&(uint64_t)a) - ((0xffffULL)&(uint64_t)b);
    return (v == 0) ? 0:(v > 0)? 1:-1;
}

inline int8_t hashmap_cmp_i32(void *a, void *b){
    uint32_t v   = ((0xffffffffULL)&(uint64_t)a)
                    - ((0xffffffffULL)&(uint64_t)b);
    return (v == 0) ? 0:(v > 0)? 1:-1;
}

inline int8_t hashmap_cmp_i64(void *a, void *b){
    uint64_t v   = (uint64_t)a - (uint64_t)b;
    return (v == 0) ? 0:(v > 0)? 1:-1;
}

inline int8_t hashmap_cmp_ptr(void *a, void *b){
    return hashmap_cmp_i64(a, b);
}

inline int8_t hashmap_cmp_str(void *a, void *b){
    int idx     = 0;
    int8_t sum  = 0;
    char *c     = (char*)a;
    char *d     = (char*)b;
    if(a == NULL || b == NULL)  {
        return 0;
    }
    while(c[idx] != 0 && d[idx] != 0)   {
        sum += (c[idx] - d[idx]);
        idx++;
    }
    return (sum == 0) ? 0:(sum > 0)? 1:-1;
}

uint32_t hashmap_hash_i8(void *val){
    return (0xffUL)&(uint64_t)val;

}

uint32_t hashmap_hash_i16(void *val){
    return (0xffffUL)&(uint64_t)val;
}

uint32_t hashmap_hash_i32(void *val){
    return (0xffffffffUL)&(uint64_t)val;
}

uint32_t hashmap_hash_i64(void *val){
    // take first 16 bytes verbatim (high variance in time-local allocations)
    // and last 16 bytes verbatim (high variance in time-dislocal allocations)
    // and hope for the best
    return ((uint64_t)val&(0xffffULL << 48)) | ((uint64_t)val&(0xffffULL));
}

uint32_t hashmap_hash_ptr(void *ptr){
    return hashmap_hash_i64(ptr);
}

uint32_t hashmap_hash_str(void *str){
    uint32_t r  = 0xcafebabe;
    int idx     = 0;
    int shift   = 0;
    if(str == NULL) {
        return 0;
    }
    while(((char*)str)[idx] != 0)    {
        r ^= ((char*)str)[idx] << shift;
        idx++;
        if(shift > 24)   {
            shift = 0;
        }
        idx++;
    }
    return r;
}

