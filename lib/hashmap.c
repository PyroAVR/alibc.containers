#include "../include/hashmap.h"
#include "../include/debug.h"
#include "../include/dynabuf.h"
#include "../include/bitmap.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Private state and types
#define kv_cast(x) ((kv_pair*)(x))
#define filter_size_constraint(x) ((x > 8) ? (x >> 3):1)

#define REHASH_RESIZE 0
#define REHASH_NEWALG 1

// Private functions
static hashmap_status rehash(hashmap_t *self, int reason);
static hashmap_status check_valid(hashmap_t *self);
static hashmap_status check_space_available(hashmap_t *self, int size);
static int hashmap_locate(hashmap_t *, void *);




hashmap_t *create_hashmap(hash_type *hashfn, cmp_type *comparefn, int size) {

    hashmap_t *r    = malloc(sizeof(hashmap_t));
    if(r == NULL)  {
        DBG_LOG("Could not malloc hashmap_t\n");
        return NULL;
    }

    r->map    = create_dynabuf(size*sizeof(kv_pair));
    if(r->map == NULL)    {
        DBG_LOG("Could not create new array for hashmap\n");
        free(r);
        return NULL;
    }
    // one bit per entry
    r->_filter  = create_bitmap(size);
    if(r->_filter == NULL)    {
        DBG_LOG("Could not create validity map for hashmap\n");
        dynabuf_free(r->map);
        free(r);
        return NULL;
    }

    memset(r->map->buf, 0, size*sizeof(kv_pair));
    memset(r->_filter->buf, 0,filter_size_constraint(size));

    r->hash     = hashfn;
    /*
     *r->load     = default_load;
     */
    r->compare  = comparefn;
    r->entries  = 0;
    r->capacity = size;

    
    return r;
}

hashmap_status hashmap_set(hashmap_t *self, void *key, void *value)    {
    hashmap_status status;
    uint32_t    hash;
    uint32_t    index;
    uint32_t    start_index = 0;
    switch((status = check_space_available(self, 1)))   {
        case SUCCESS:
            hash        = self->hash(key);
            index       = hash % self->capacity;
            start_index = index;
            // index guaranteed in range
            // scan for next open entry
            while(bitmap_contains(self->_filter, index))    {
                index = (index + 1) % self->capacity;
                if(index == start_index)    {
                    DBG_LOG("hashmap was resized on key at:%p\n", key);
                    switch((status = rehash(self, REHASH_RESIZE)))  {
                        case SUCCESS:
                            return hashmap_set(self, key, value);
                        break;
                        default:
                            DBG_LOG("Could not rehash on insert\n");
                            return status;
                    }
                }
            }
            kv_pair a = { .key = key, .value = value};
            kv_cast(self->map->buf)[index]  = a;
            bitmap_add(self->_filter, index);

            self->entries++;
        break;

        case NO_MEM:
            DBG_LOG("hashmap was resized on key at:%p\n", key);
            switch((status = rehash(self, REHASH_RESIZE)))   {
                case SUCCESS:
                    return hashmap_set(self, key, value);
                break;
                default:
                    DBG_LOG("Could not resize hashmap buffer\n");
                break;
            }

        default:
            DBG_LOG("check_valid returned invalid status: %d\n", status);
        break;
    }
    return status;
}


void *hashmap_fetch(hashmap_t *self, void *key) {
    hashmap_status  status;
    int             key_index;
    switch((status = check_valid(self)))    {
        case SUCCESS:
            key_index = hashmap_locate(self, key);
            if(key_index != -1) {
                return kv_cast(self->map->buf)[key_index].value;
            }
            else    {
                return NULL;
            }
        break;

        default:
            DBG_LOG("check_valid returned invalid status: %d\n", status);
            return NULL;
    }
}


void *hashmap_remove(hashmap_t *self, void *key)  {
    hashmap_status  status;
    int             key_index;
    void            *value;
    switch((status = check_valid(self)))    {
        case SUCCESS:
            key_index = hashmap_locate(self, key);
            if(key_index != -1) {
                value   = kv_cast(self->map->buf)[key_index].value;
                bitmap_remove(self->_filter, key_index);
                return value;
            }
            else    {
                return NULL;
            }
        break;

        default:
            DBG_LOG("check_valid returned invalid status: %d\n", status);
            return NULL;
    }
}

uint32_t hashmap_size(hashmap_t *self)   {
    hashmap_status status;
    switch((status = check_valid(self)))   {
        case SUCCESS:
            return self->entries;
        break;
        default:
            DBG_LOG("Invalid status returned from check_valid: %d\n", status);
            return 0;
    }
}

void hashmap_free(hashmap_t *self)   {
    int status;
    switch((status = check_valid(self)))    {
        case SUCCESS:
            dynabuf_free(self->map);
            bitmap_free(self->_filter);
        case NULL_IMPL:
            free(self);
        break;
        default:
            DBG_LOG("Attempted to free an invalid hashmap\n");
        break;
    }
}

/*
 * Helper functions
 */
static int hashmap_locate(hashmap_t *self, void *key)  {
    if(check_valid(self) != SUCCESS)    {
        return -1;
    }

    uint32_t    hash        = self->hash(key);
    uint32_t    index       = hash % self->capacity;
    uint32_t    start_index = index;
    int         is_valid    = 0;
    int         is_equal    = 0;
    while(!is_equal || !is_valid)   {
        is_equal = self->compare(key, kv_cast(self->map->buf)[index].key) == 0;
        is_valid = bitmap_contains(self->_filter, index) != 0;
        if(is_equal && is_valid)    {
            break;
        }
        
        index = (index + 1) % self->capacity;
        if(index == start_index)    {
            index = -1;
            break;
        }

    }
    return index;
}
// try to rewrite this to recursively re-hash in place via marking each
// kv pair as whether they're in the correct location or not, and rehashing in
// storage order until a conflict occurs - then recursively re-hash until no
// conflicts remain and continue until the end of the list is reached
static hashmap_status rehash(hashmap_t *self, int reason)   {
    hashmap_status status;
    dynabuf_t *scratch_map;
    dynabuf_t *scratch_filter;
    uint32_t new_size;
    switch((status = check_valid(self)))    {
        case SUCCESS:
            switch(reason)  {
                case REHASH_RESIZE:
                    // TODO: make this pluggable
                    new_size    = 2*self->capacity + 1;
                break;
                case REHASH_NEWALG:
                    new_size    = self->capacity;
                break;
                default:
                    DBG_LOG("Unknown rehash reason %d ignored.\n", reason);
                    return BAD_PARAM;
            }
        break;
        default:
            DBG_LOG("Invalid status returned from check_valid: %d\n", status);
            return status;
    }

    scratch_map     = create_dynabuf(new_size*sizeof(kv_pair));
    if(scratch_map == NULL) {
        DBG_LOG("Could not create new array with size %d\n",
                self->capacity);
        return NO_MEM;
    }
    
    scratch_filter  = create_bitmap(new_size);
    if(scratch_filter == NULL) {
        DBG_LOG("Could not create new array with size %d\n",
                self->capacity);
        dynabuf_free(scratch_map);
        return NO_MEM;
    }

    // clear out the new buffer
    memset(scratch_map->buf, 0, new_size*sizeof(kv_pair));
    memset(scratch_filter->buf, 0, filter_size_constraint(new_size));

    for(uint32_t i = 0; i < self->capacity; i++)    {
        if(!bitmap_contains(self->_filter, i))   {
            continue;
        }
        uint32_t hash   = self->hash(kv_cast(self->map->buf)[i].key);
        uint32_t index  = hash % new_size;
        uint32_t start_index = index;
        // scan for next open entry
        // the condition here is the same as kv_valid, but as we do not have
        // a valid 'self' for this entry, we have to re-write it here
        // FIXME this should not be the case.
        while(bitmap_contains(scratch_filter, index))  {
            index = (index + 1) % self->capacity;
            if(index == start_index)    {
                DBG_LOG("Could not find a suitable location "
                " to insert hash: %d and new size: %d\n",
                hash, new_size);
                dynabuf_free(scratch_map);
                bitmap_free(scratch_filter);
                return NO_MEM;
            }
        }
        kv_cast(scratch_map->buf)[index] = kv_cast(self->map->buf)[i];
        bitmap_add(scratch_filter, index);
    }

    dynabuf_free(self->map);
    bitmap_free(self->_filter);
    self->map       = scratch_map;
    self->_filter   = scratch_filter;
    self->capacity  = new_size;
    return SUCCESS;
}

// possible returns: SUCCESS NULL_ARG NULL_IMPL NULL_ARRAY NULL_HASH NULL_LOAD
hashmap_status check_valid(hashmap_t *self)    {
    if(self == NULL)    {
        return NULL_ARG;
    }
    if(self->map == NULL) {
        return NULL_BUF;
    }
    if(self->hash == NULL)     {
        return NULL_HASH;
    }
    /*
     *if(self->load == NULL) {
     *    return NULL_LOAD;
     *}
     */
    return SUCCESS;
}

hashmap_status check_space_available(hashmap_t *self, int size)  {
    hashmap_status status;
    switch((status = check_valid(self))) {
        case SUCCESS:
            return ((self->entries + size) <= self->capacity) ? SUCCESS:NO_MEM;
        break;
        default:
            DBG_LOG("Invalid status returned from check_valid: %d\n", status);
            return status;
    }
}


/*
 *uint32_t default_load(hashmap_t *self)  {
 *    hashmap_status status;
 *    switch((status = check_valid(self)))   {
 *        case SUCCESS:
 *            return (uint32_t)((float)(self->entries)
 *                             / (float)(self->capacity));
 *        break;
 *        default:
 *            DBG_LOG("Invalid status returned from check_valid: %d\n", status);
 *            return 0;
 *        break;
 *    }
 *}
 */
/*
 *static void dump_table(hashmap_t *self) {
 *    for(int i = 0; i < self->capacity; i++) {
 *        printf("%s:%d\t\t%d\n", kv_cast(self->map->buf)[i].key, kv_cast(self->map->buf)[i].value, kv_valid(self, i) ? 1:0);
 *    }
 *    puts("\n");
 *}
 */
