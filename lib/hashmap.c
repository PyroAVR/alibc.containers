#include <alibc/extensions/hashmap.h>
#include <alibc/extensions/debug.h>
#include <alibc/extensions/dynabuf.h>
#include <alibc/extensions/bitmap.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Private state and types
#define kv_cast(x) ((kv_pair*)(x))
#define filter_size_constraint(x) ((x > 8) ? (x >> 3):1)


// Private functions
static hashmap_status rehash(hashmap_t *self, int count);
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
    r->status   = SUCCESS;
    
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
                    DBG_LOG("hashmap resize on key at:%p"
                            " was unexpected.  Corruption likely.\n", key);
                    status = rehash(self, 2*self->capacity + 1);
                    if(status != SUCCESS) {
                        DBG_LOG("Could not rehash on insert\n");
                        self->status = status;
                        return status;
                    }
                    return hashmap_set(self, key, value);
                }
            }
            kv_pair a = { .key = key, .value = value};
            kv_cast(self->map->buf)[index]  = a;
            bitmap_add(self->_filter, index);
            self->entries++;
        break;

        case NO_MEM:
            DBG_LOG("hashmap was resized on key at:%p\n", key);
            status = rehash(self, 2*self->capacity + 1);
            if(status != SUCCESS) {
                DBG_LOG("Could not resize hashmap buffer\n");
                status = NO_MEM;
                return status;
            }
            return hashmap_set(self, key, value);
        break;

        default:
            DBG_LOG("check_valid returned invalid status: %d\n", status);
        break;
    }
    self->status = status;
    return status;
}


void *hashmap_fetch(hashmap_t *self, void *key) {
    hashmap_status  status;
    int             key_index;
    switch((status = check_valid(self)))    {
        case SUCCESS:
            key_index = hashmap_locate(self, key);
            if(key_index != -1) {
                self->status = SUCCESS;
                return kv_cast(self->map->buf)[key_index].value;
            }
            else    {
                self->status = IDX_OOB; //fixme
                return NULL;
            }
        break;

        default:
            DBG_LOG("check_valid returned invalid status: %d\n", status);
            self->status = status;
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

int hashmap_resize(hashmap_t *self, int count) {
    int status = SUCCESS;
    if(check_valid(self) != SUCCESS) {
        status = NULL_IMPL;
        goto finish;
    }

    if(count > self->entries) {
        status = rehash(self, count);
        if(status != SUCCESS) {
            DBG_LOG("Could not rehash to new size %d\n", count);
            status = NO_MEM;
            goto finish;
        }
    }
    else if(count < self->entries) {
        DBG_LOG("Requested count %d was too small.\n", count);
        status = IDX_OOB;
        goto finish;
    }

finish:
    self->status = status;
    return status;
}
uint32_t hashmap_size(hashmap_t *self)   {
    hashmap_status status;
    switch((status = check_valid(self)))   {
        case SUCCESS:
            self->status = status;
            return self->entries;
        break;
        default:
            DBG_LOG("Invalid status returned from check_valid: %d\n", status);
            self->status = status;
            return -1;
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

inline int hashmap_okay(hashmap_t *self)   {
    if(check_valid(self) != SUCCESS) {
        return NULL_ARG;
    }
    return self->status;
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
        uint8_t null_check = 0;
        
        is_valid = bitmap_contains(self->_filter, index) != 0;
        if(is_valid) {
            null_check  = kv_cast(self->map->buf)[index].key == NULL;
            null_check  |= ((key == NULL) << 1);
            switch(null_check) {
                case 0: // neither is null
                    is_equal = self->compare(key,
                                   kv_cast(self->map->buf)[index].key) == 0;
                break;

                case 1:
                case 2: // exclusive null cases, wrong index.
                    is_equal = 0;
                break;

                case 3: // both are null
                    is_equal = 1;
                break;
            }
        }

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
static hashmap_status rehash(hashmap_t *self, int count)   {
    hashmap_status status;
    dynabuf_t *scratch_map;
    dynabuf_t *scratch_filter;
    if((status = check_valid(self)) != SUCCESS) {
        DBG_LOG("Invalid status returned from check_valid: %d\n", status);
        status = NULL_IMPL;
        goto finish;
    }

    scratch_map     = create_dynabuf(count*sizeof(kv_pair));
    if(scratch_map == NULL) {
        DBG_LOG("Could not create new array with size %d\n",
                self->capacity);
        status = NO_MEM;
        goto finish;
    }
    
    scratch_filter  = create_bitmap(count);
    if(scratch_filter == NULL) {
        DBG_LOG("Could not create new array with size %d\n",
                self->capacity);
        dynabuf_free(scratch_map);
        status = NO_MEM;
        goto finish;
    }

    // clear out the new buffer
    memset(scratch_map->buf, 0, count*sizeof(kv_pair));
    memset(scratch_filter->buf, 0, filter_size_constraint(count));

    for(uint32_t i = 0; i < self->capacity; i++)    {
        if(!bitmap_contains(self->_filter, i))   {
            continue;
        }
        uint32_t hash   = self->hash(kv_cast(self->map->buf)[i].key);
        uint32_t index  = hash % count;
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
                hash, count);
                dynabuf_free(scratch_map);
                bitmap_free(scratch_filter);
                status = NO_MEM;
                goto finish;
            }
        }
        kv_cast(scratch_map->buf)[index] = kv_cast(self->map->buf)[i];
        bitmap_add(scratch_filter, index);
    }

    dynabuf_free(self->map);
    bitmap_free(self->_filter);
    self->map       = scratch_map;
    self->_filter   = scratch_filter;
    self->capacity  = count;
finish:
    return status;
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
