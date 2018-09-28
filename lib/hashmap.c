#include "../include/hashmap.h"
#include "../include/debug.h"
#include "../include/array.h"
#include <stdlib.h>

// Public functions
static void hashmap_set(proto_map *self, void *key, void *value);
static void *hashmap_get(proto_map *self, void *key);
static void hashmap_remove(proto_map *self, void *key);
static uint32_t hashmap_size(proto_map *self);
static void free_hashmap(proto_map *self);


// Private state and types
#define kv_cast(x) ((kv_pair*)(x))
typedef struct {
    void *key;
    void *value;
} kv_pair;

#define impl_cast(x) ((hashmap_impl_t*)(x))
typedef struct  {
    proto_list *map;
    hash_type *hash;
    load_type *load;
    cmp_type  *compare;
    uint32_t entries;
    uint32_t capacity;
} hashmap_impl_t;

typedef enum    {
    SUCCESS,
    NULL_ARG,
    NULL_IMPL,
    NULL_ARRAY,
    NULL_ENTRY,
    NULL_HASH,
    NULL_LOAD,
    IDX_OOB,
    NO_MEM,
    BAD_PARAM
} hashmap_status;

#define starting_size 16
#define REHASH_RESIZE 0
#define REHASH_NEWALG 1

// Private functions
static uint32_t default_load(proto_map *self);
static hashmap_status rehash(proto_map *self, int reason);
static hashmap_status check_valid(proto_map *self);
static hashmap_status check_space_available(proto_map *self, int size);
static kv_pair *kv_malloc(void *, void*);



proto_map *create_hashmap(hash_type *hashfn) {
    proto_map *r = malloc(sizeof(proto_map));
    if(r == NULL)    {
        DBG_LOG("Could not malloc proto_map\n");
        return NULL;
    }

    r->impl     = malloc(sizeof(hashmap_impl_t));
    if(r->impl == NULL)  {
        DBG_LOG("Could not malloc hashmap_impl_t\n");
        free(r);
        return NULL;
    }

    impl_cast(r->impl)->map    = create_array(starting_size);
    if(impl_cast(r->impl)->map == NULL)    {
        DBG_LOG("Could not create new array for hashmap\n");
        free(r->impl);
        free(r);
        return NULL;
    }

    impl_cast(r->impl)->hash    = hashfn;
    impl_cast(r->impl)->load    = default_load;
    impl_cast(r->impl)->entries = 0;

    r->set      = hashmap_set;
    r->fetch    = hashmap_get;
    r->size     = hashmap_size;
    r->free     = free_hashmap;
    
    return r;
}

static void hashmap_set(proto_map *self, void *key, void *value)    {
    hashmap_impl_t *impl;
    hashmap_status status;
    uint32_t hash;
    uint32_t index;
    kv_pair *to_insert = NULL;
    switch((status = check_space_available(self, 1)))   {
        case SUCCESS:
            impl    = impl_cast(self->impl);
            hash    = impl->hash(key);
            index   = hash/impl->capacity;
            // scan for next open entry
            while(impl->map->fetch(impl->map, index) != NULL)    {
                index++;    
                if(index > impl->map->size(impl->map))  {
                    rehash(self, REHASH_RESIZE);
                    return hashmap_set(self, key, value);
                }
            }
            to_insert   = kv_malloc(key, value);
            if(to_insert == NULL)   {
                DBG_LOG("Could not malloc key-value pair\n");
                return;
            }
            impl->map->insert(impl->map, index, to_insert);
            impl->entries++;
        break;

        case NO_MEM:
            rehash(self, REHASH_RESIZE);
            return hashmap_set(self, key, value);
            break;

        default:
            DBG_LOG("check_valid returned invalid status: %d\n", status);
            return;
    }
}

static void *hashmap_get(proto_map *self, void *key) {

}

static uint32_t hashmap_size(proto_map *self)   {
    hashmap_impl_t *impl;
    hashmap_status status;
    switch((status = check_valid(self)))   {
        case SUCCESS:
            impl    = impl_cast(self->impl);
            return impl->entries;
        break;
        default:
            DBG_LOG("Invalid status returned from check_valid: %d\n", status);
            return 0;
    }
}

static void free_hashmap(proto_map *self)   {
    hashmap_impl_t *impl;
    int status;
    switch((status = check_valid(self)))    {
        case SUCCESS:
            impl    = impl_cast(self->impl);
            impl->map->free(impl->map);
            free(impl->map);
        case NULL_ARRAY:
            free(impl);
        case NULL_IMPL:
            free(self);

    }
}

/*
 * Helper functions
 */

static hashmap_status rehash(proto_map *self, int reason)   {
    hashmap_impl_t *impl;
    hashmap_status status;
    proto_list *scratch;
    uint32_t new_size;
    switch((status = check_valid(self)))    {
        case SUCCESS:
            impl    = impl_cast(self->impl);
            switch(reason)  {
                case REHASH_RESIZE:
                    // TODO: make this pluggable
                    new_size    = 2*impl->capacity + 1;
                    scratch     = create_array(new_size);

                break;
                case REHASH_NEWALG:
                    new_size    = impl->capacity;
                    scratch     = create_array(new_size);

                break;
                default:
                    DBG_LOG("Unknown rehash reason %d ignored.\n", reason);
                    return BAD_PARAM;
            }

            if(scratch == NULL) {
                DBG_LOG("Could not create new array with size %d\n",
                        impl->capacity);
                return NO_MEM;
            }
            for(uint32_t i = 0; i < new_size; i++)  {
                scratch->insert(scratch, i, 0);
            }
            for(uint32_t i = 0; i < impl->capacity; i++)    {
                uint32_t hash   = impl->hash(impl->map->fetch(impl->map, i));
                uint32_t index  = hash/impl->capacity;
                while(scratch->fetch(scratch, index) != NULL)  {
                    index++;
                    if(index > new_size - 1)    {
                        DBG_LOG(
                        "Could not find a suitable location to insert hash: %d"
                        " and new size: %d\n", hash, new_size);
                        scratch->free(scratch);
                        return NO_MEM;
                    }
                }
                    scratch->insert(scratch, index,
                                    impl->map->fetch(impl->map, i));
            }
        break;
        default:
            DBG_LOG("Invalid status returned from check_valid: %d\n", status);
            return status;
    }
}

// possible returns: SUCCESS NULL_ARG NULL_IMPL NULL_ARRAY NULL_HASH NULL_LOAD
static hashmap_status check_valid(proto_map *self)    {
    if(self == NULL)    {
        return NULL_ARG;
    }
    
    if(self->impl == NULL)  {
        return NULL_IMPL;
    }

    if(impl_cast(self->impl)->map == NULL) {
        return NULL_ARRAY;
    }
    if(impl_cast(self->impl)->hash == NULL)     {
        return NULL_HASH;
    }
    if(impl_cast(self->impl)->load == NULL) {
        return NULL_LOAD;
    }
    return SUCCESS;
}

static hashmap_status check_space_available(proto_map *self, int size)  {
    hashmap_impl_t *impl;
    hashmap_status status;
    switch((status = check_valid(self))) {
        case SUCCESS:
            impl    = impl_cast(self->impl);
            return ((impl->entries + size) < impl->capacity) ? SUCCESS:NO_MEM;
        break;
        default:
            DBG_LOG("Invalid status returned from check_valid: %d\n", status);
            return status;
    }
}


static kv_pair *kv_malloc(void *key, void *value)  {
    kv_pair *r = malloc(sizeof(kv_pair));
    if(r == NULL)   {
        return NULL;
    }   else    {
        r->key      = key;
        r->value    = value;
        return r;
    }
}

#define hash_seed 0xcafebabe
static uint32_t default_hash(void *data, uint32_t size) {
    if(data == NULL)    {
        DBG_LOG("Null pointer encountered in default_hash\n");
        return -1;
    }
    uint32_t hash = hash_seed;
    for(uint32_t i = 0; i < size; i++)  {
        hash ^= ((uint8_t*)data)[i];
    }
    return hash;
}

static uint32_t default_load(proto_map *self)  {
    hashmap_impl_t *impl;
    hashmap_status status;
    switch((status = check_valid(self)))   {
        case SUCCESS:
            impl    = impl_cast(self->impl);
            return (uint32_t)((float)(impl->entries)
                             / (float)(impl->capacity));
        break;
        default:
            DBG_LOG("Invalid status returned from check_valid: %d\n", status);
            return 0;
        break;
    }
}
