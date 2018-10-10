#include "../include/hashmap.h"
#include "../include/debug.h"
#include "../include/dynabuf.h"
#include <stdlib.h>
#include <string.h>

// Private state and types
#define kv_cast(x) ((kv_pair*)(x))


#define starting_size 8
#define REHASH_RESIZE 0
#define REHASH_NEWALG 1

// Private functions
static hashmap_status rehash(hashmap_t *self, int reason);
static hashmap_status check_valid(hashmap_t *self);
static hashmap_status check_space_available(hashmap_t *self, int size);
static kv_pair *kv_malloc(void *, void*);



hashmap_t *create_hashmap(hash_type *hashfn, cmp_type *comparefn) {

    hashmap_t *r    = malloc(sizeof(hashmap_t));
    if(r == NULL)  {
        DBG_LOG("Could not malloc hashmap_t\n");
        return NULL;
    }

    r->map    = create_dynabuf(starting_size*sizeof(void*));
    if(r->map == NULL)    {
        DBG_LOG("Could not create new array for hashmap\n");
        free(r);
        return NULL;
    }

    memset(r->map->buf, 0, starting_size);

    r->hash     = hashfn;
    /*
     *r->load     = default_load;
     */
    r->compare  = comparefn;
    r->entries  = 0;
    r->capacity = starting_size;

    
    return r;
}

hashmap_status hashmap_set(hashmap_t *self, void *key, void *value)    {
    hashmap_status status;
    uint32_t    hash;
    uint32_t    index;
    uint32_t    start_index = 0;
    uint8_t     have_looped = 0;
    kv_pair *to_insert = NULL;
    switch((status = check_space_available(self, 1)))   {
        case SUCCESS:
            hash        = self->hash(key);
            index       = hash % self->capacity;
            start_index = index;
            // index guaranteed in range
            // scan for next open entry
            while(self->map->buf[index] != NULL)    {
                switch(have_looped) {
                    case 0:
                    if(index > self->capacity)  {
                        have_looped = 1;
                        index = 0;
                    }
                    break;
                    default:
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
                index++;    
            }

            to_insert   = kv_malloc(key, value);
            if(to_insert == NULL)   {
                DBG_LOG("Could not malloc key-value pair\n");
                return NO_MEM;
            }
            self->map->buf[index]   = to_insert;
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
                    return status;
                break;
            }

        default:
            DBG_LOG("check_valid returned invalid status: %d\n", status);
            return status;
    }
}

void *hashmap_fetch(hashmap_t *self, void *key) {
    hashmap_status  status;
    uint32_t        hash;
    uint32_t        index;
    kv_pair         *candidate = NULL;
    switch((status = check_valid(self)))    {
        case SUCCESS:
            hash    = self->hash(key);
            index   = hash % self->capacity;
            do  {
                candidate   = (kv_pair*)self->map->buf[index];
                index++;
            }   while(candidate != NULL
                      && self->compare(key, candidate->key) != 0
                      && (index - 1) < self->capacity);

            return ((void*)candidate == NULL) ? NULL:candidate->value;
        break;

        default:
            DBG_LOG("check_valid returned invalid status: %d\n", status);
            return NULL;
    }
}


void *hashmap_remove(hashmap_t *self, void *key)  {
    hashmap_status  status;
    uint32_t        hash;
    uint32_t        index;
    kv_pair         *candidate;
    switch((status = check_valid(self)))    {
        case SUCCESS:
            hash    = self->hash(key);
            index   = hash % self->capacity;
            do  {
                candidate   = (kv_pair*)self->map->buf[index];
                index++;
            }   while(self->compare(key, (void*)candidate) != 0);
            self->map->buf[--index] = NULL;
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
    void *current_item = NULL;
    switch((status = check_valid(self)))    {
        case SUCCESS:
            for(int i = 0; i < self->capacity; i++)   {
                if((current_item = self->map->buf[i]) != NULL)  {
                   free(current_item); 
                }
            }
            dynabuf_free(self->map);
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
// try to rewrite this to recursively re-hash in place via marking each
// kv pair as whether they're in the correct location or not, and rehashing in
// storage order until a conflict occurs - then recursively re-hash until no
// conflicts remain and continue until the end of the list is reached
static hashmap_status rehash(hashmap_t *self, int reason)   {
    hashmap_status status;
    dynabuf_t *scratch;
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

            scratch     = create_dynabuf(new_size*sizeof(void*));
            if(scratch == NULL) {
                DBG_LOG("Could not create new array with size %d\n",
                        self->capacity);
                return NO_MEM;
            }
            memset(scratch->buf, 0, new_size);
            for(uint32_t i = 0; i < self->capacity; i++)    {
                if(self->map->buf[i] == NULL)   {
                    continue;
                }
                uint32_t hash   = self->hash(kv_cast(self->map->buf[i])->key);
                uint32_t index  = hash % new_size;
                uint32_t start_index = index;
                uint8_t  have_looped = 0;
                while(scratch->buf[index] != NULL)  {
                    switch(have_looped) {
                        case 0:
                            if(index > new_size - 1)    {
                                have_looped = 1;
                                index = 0;
                            }
                        break;
                        default:
                            if(index == start_index)    {
                                DBG_LOG(
                                "Could not find a suitable location "
                                " to insert hash: %d and new size: %d\n",
                                hash, new_size);
                                dynabuf_free(scratch);
                                return NO_MEM;
                            }
                        break;
                    }
                    index++;
                }
                    scratch->buf[index] = self->map->buf[i];
            }
            dynabuf_free(self->map);
            self->map = scratch;
            self->capacity = new_size;
        break;
        default:
            DBG_LOG("Invalid status returned from check_valid: %d\n", status);
            return status;
    }
    return SUCCESS;
}

// possible returns: SUCCESS NULL_ARG NULL_IMPL NULL_ARRAY NULL_HASH NULL_LOAD
hashmap_status check_valid(hashmap_t *self)    {
    if(
            self == NULL)    {
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
            return ((self->entries + size) < self->capacity) ? SUCCESS:NO_MEM;
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
