#include "../include/hashmap.h"
#include "../include/debug.h"
#include "../include/dynabuf.h"
#include <stdlib.h>
#include <string.h>

// Public functions
static void hashmap_set(proto_map *self, void *key, void *value);
static void *hashmap_get(proto_map *self, void *key);
static void *hashmap_remove(proto_map *self, void *key);
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
    dynabuf_t *map;
    hash_type *hash;
    load_type *load;
    cmp_type  *compare;
    uint32_t entries;
    uint32_t capacity;
} hashmap_impl_t;

#define starting_size 8
#define REHASH_RESIZE 0
#define REHASH_NEWALG 1

// Private functions
static uint32_t default_load(proto_map *self);
static hashmap_status rehash(proto_map *self, int reason);
static hashmap_status check_valid(proto_map *self);
static hashmap_status check_space_available(proto_map *self, int size);
static kv_pair *kv_malloc(void *, void*);



proto_map *create_hashmap(hash_type *hashfn, cmp_type *comparefn) {
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

    impl_cast(r->impl)->map    = create_dynabuf(starting_size*sizeof(void*));
    if(impl_cast(r->impl)->map == NULL)    {
        DBG_LOG("Could not create new array for hashmap\n");
        free(r->impl);
        free(r);
        return NULL;
    }

    memset(impl_cast(r->impl)->map->buf, 0, starting_size);

    impl_cast(r->impl)->hash    = hashfn;
    impl_cast(r->impl)->load    = default_load;
    impl_cast(r->impl)->compare = comparefn;
    impl_cast(r->impl)->entries = 0;
    impl_cast(r->impl)->capacity = starting_size;

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
            index   = hash % impl->capacity;
            // index guaranteed in range
            // scan for next open entry
            // TODO: should we wrap to lower addresses before rehashing?
            while(impl->map->buf[index] != NULL)    {
                if(index > impl->capacity)  {
                    DBG_LOG("hashmap was resized on key at:%p\n", key);
                    rehash(self, REHASH_RESIZE);
                    return hashmap_set(self, key, value);
                }
                index++;    
            }

            to_insert   = kv_malloc(key, value);
            if(to_insert == NULL)   {
                DBG_LOG("Could not malloc key-value pair\n");
                return;
            }
            impl->map->buf[index]   = to_insert;
            impl->entries++;
        break;

        case NO_MEM:
            DBG_LOG("hashmap was resized on key at:%p\n", key);
            rehash(self, REHASH_RESIZE);
            return hashmap_set(self, key, value);
            break;

        default:
            DBG_LOG("check_valid returned invalid status: %d\n", status);
            return;
    }
}

static void *hashmap_get(proto_map *self, void *key) {
    hashmap_impl_t  *impl;
    hashmap_status  status;
    uint32_t        hash;
    uint32_t        index;
    kv_pair         *candidate = NULL;
    switch((status = check_valid(self)))    {
        case SUCCESS:
            impl    = impl_cast(self->impl);
            hash    = impl->hash(key);
            index   = hash % impl->capacity;
            do  {
                candidate   = (kv_pair*)impl->map->buf[index];
                index++;
            }   while(candidate != NULL
                      && impl->compare(key, candidate->key) != 0
                      && (index - 1) < impl->capacity);

            return ((void*)candidate == NULL) ? NULL:candidate->value;
        break;

        default:
            DBG_LOG("check_valid returned invalid status: %d\n", status);
            return NULL;
    }
}


static void *hashmap_remove(proto_map *self, void *key)  {
    hashmap_impl_t  *impl;
    hashmap_status  status;
    uint32_t        hash;
    uint32_t        index;
    kv_pair         *candidate;
    switch((status = check_valid(self)))    {
        case SUCCESS:
            impl    = impl_cast(self->impl);
            hash    = impl->hash(key);
            index   = hash % impl->capacity;
            do  {
                candidate   = (kv_pair*)impl->map->buf[index];
                index++;
            }   while(impl->compare(key, (void*)candidate) != 0);
            impl->map->buf[--index] = NULL;
        break;

        default:
            DBG_LOG("check_valid returned invalid status: %d\n", status);
            return NULL;
    }
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
    void *current_item = NULL;
    switch((status = check_valid(self)))    {
        case SUCCESS:
            impl    = impl_cast(self->impl);
            for(int i = 0; i < impl->capacity; i++)   {
                if((current_item = impl->map->buf[i]) != NULL)  {
                   free(current_item); 
                }
            }
            dynabuf_free(impl->map);
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
// try to rewrite this to recursively re-hash in place via marking each
// kv pair as whether they're in the correct location or not, and rehashing in
// storage order until a conflict occurs - then recursively re-hash until no
// conflicts remain and continue until the end of the list is reached
static hashmap_status rehash(proto_map *self, int reason)   {
    hashmap_impl_t *impl;
    hashmap_status status;
    dynabuf_t *scratch;
    uint32_t new_size;
    switch((status = check_valid(self)))    {
        case SUCCESS:
            impl    = impl_cast(self->impl);
            switch(reason)  {
                case REHASH_RESIZE:
                    // TODO: make this pluggable
                    new_size    = 2*impl->capacity + 1;
                break;
                case REHASH_NEWALG:
                    new_size    = impl->capacity;
                break;
                default:
                    DBG_LOG("Unknown rehash reason %d ignored.\n", reason);
                    return BAD_PARAM;
            }

            scratch     = create_dynabuf(new_size*sizeof(void*));
            if(scratch == NULL) {
                DBG_LOG("Could not create new array with size %d\n",
                        impl->capacity);
                return NO_MEM;
            }
            memset(scratch->buf, 0, new_size);
            for(uint32_t i = 0; i < impl->capacity; i++)    {
                if(impl->map->buf[i] == NULL)   {
                    continue;
                }
                uint32_t hash   = impl->hash(kv_cast(impl->map->buf[i])->key);
                uint32_t index  = hash % new_size;
                while(scratch->buf[index] != NULL)  {
                    index++;
                    if(index > new_size - 1)    {
                        DBG_LOG(
                        "Could not find a suitable location to insert hash: %d"
                        " and new size: %d\n", hash, new_size);
                        dynabuf_free(scratch);
                        return NO_MEM;
                    }
                }
                    scratch->buf[index] = impl->map->buf[i];
            }
            impl->map = scratch;
            impl->capacity = new_size;
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
