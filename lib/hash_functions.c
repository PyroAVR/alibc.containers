#include <stdint.h>
#include <stddef.h>

uint32_t alc_default_hash_i8(void *val){
    return (0xffUL)&(uint64_t)val;

}

uint32_t alc_default_hash_i16(void *val){
    return (0xffffUL)&(uint64_t)val;
}

uint32_t alc_default_hash_i32(void *val){
    return (0xffffffffUL)&(uint64_t)val;
}

uint32_t alc_default_hash_i64(void *val){
    // take first 16 bytes verbatim (high variance in time-local allocations)
    // and last 16 bytes verbatim (high variance in time-dislocal allocations)
    // and hope for the best
    return ((uint64_t)val&(0xffffULL << 48)) | ((uint64_t)val&(0xffffULL));
}

uint32_t alc_default_hash_ptr(void *ptr){
    return alc_default_hash_i64(ptr);
}

uint32_t alc_default_hash_str(void *str){
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

