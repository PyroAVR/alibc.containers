#include <alibc/containers/hashmap.h>
#include <alibc/containers/debug.h>
#include <alibc/containers/dynabuf.h>
#include <alibc/containers/bitmap.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Private state and types
#define value_at(self, idx) (void**)((char*)dynabuf_fetch(self->map, idx) + self->val_offset)
#define key_at(self, idx) (void**)((char*)dynabuf_fetch(self->map, idx))
#define filter_size_constraint(x) ((x > 8) ? (x >> 3):1)


// Private functions
static int rehash(hashmap_t *self, int count);
static int check_valid(hashmap_t *self);
static int check_space_available(hashmap_t *self, int size);
static int hashmap_locate(hashmap_t *, void *);
static inline bool default_load(int, int);




hashmap_t *create_hashmap(int size, int keysz, int valsz, hash_type *hashfn,
        cmp_type *comparefn, load_type loadfn) {

    hashmap_t *r = malloc(sizeof(hashmap_t));
    if(r == NULL)  {
        DBG_LOG("Could not malloc hashmap_t\n");
        goto done;
    }

    r->map    = create_dynabuf(size, keysz + valsz);
    if(r->map == NULL)    {
        DBG_LOG("Could not create new array for hashmap\n");
        free(r);
        goto done;
    }
    r->_filter  = create_bitmap(size);
    if(r->_filter == NULL)    {
        DBG_LOG("Could not create validity map for hashmap\n");
        dynabuf_free(r->map);
        free(r);
        goto done;
    }

    memset(r->map->buf, 0, size*(keysz + valsz));
    memset(r->_filter->buf, 0, filter_size_constraint(size));

    r->hash     = hashfn;

    if(loadfn == NULL) {
        r->load     = default_load;
    }
    else {
        r->load     = loadfn;
    }

    r->val_offset = keysz;
    r->compare  = comparefn;
    r->entries  = 0;
    r->capacity = size;
    r->status   = ALC_HASHMAP_SUCCESS;
    
done:
    return r;
}

int hashmap_set(hashmap_t *self, void *key, void *value)    {
    int status;
    uint32_t hash;
    int index;

    switch((status = check_space_available(self, 1)))   {
        case ALC_HASHMAP_SUCCESS:
            hash        = self->hash(key);
            index       = hash % self->capacity;
            // index guaranteed in range
            // scan for next open entry
            while(bitmap_contains(self->_filter, index))    {
                if(self->compare(key, *key_at(self, index)) == 0) {
                    DBG_LOG("got repeat key case\n");
                    goto repeat_key;
                }
                index = (index + 1) % self->capacity;
            }
            self->entries++;
            int next = 0;
repeat_key:
            next = dynabuf_set_seq(self->map, index, 0, key, self->val_offset);
            dynabuf_set_seq(self->map, index, next, value, self->map->elem_size - self->val_offset);
            bitmap_add(self->_filter, index);
        break;

        case ALC_HASHMAP_NO_MEM:
            DBG_LOG("hashmap was resized on key at:%p\n", key);
            status = rehash(self, 2*self->capacity + 1);
            if(status != ALC_HASHMAP_SUCCESS) {
                DBG_LOG("Could not resize hashmap buffer\n");
                status = ALC_HASHMAP_NO_MEM;
                goto done;
            }
            status = hashmap_set(self, key, value);
            goto done;
        break;

        default:
            DBG_LOG("check_valid returned invalid status: %d\n", status);
            goto invalid_status;
        break;
    }

    if(self->load(self->entries, self->capacity) != 0) {
        status = hashmap_resize(self, 2*self->capacity + 1);
    }
done:
    self->status = status;
invalid_status:
    return status;
}


void **hashmap_fetch(hashmap_t *self, void *key) {
    int status = check_valid(self);
    int key_index;
    void **r = NULL;
    if(status != ALC_HASHMAP_SUCCESS) {
        DBG_LOG("hashmap was invalid on fetch operation.\n");
        goto invalid_status;
    }
    key_index = hashmap_locate(self, key);
    if(key_index != -1) {
        r = value_at(self, key_index);
    }
    else    {
        status = ALC_HASHMAP_NOTFOUND;
    }
done:
    self->status = status;
invalid_status:
    return r;
}


void **hashmap_remove(hashmap_t *self, void *key)  {
    int status = check_valid(self);
    int key_index;
    void **r = NULL;
    if(status != ALC_HASHMAP_SUCCESS) {
        DBG_LOG("check_valid returned invalid status: %d\n", status);
        goto invalid_status;
    }
    key_index = hashmap_locate(self, key);
    if(key_index != -1) {
        r = value_at(self, key_index);
        bitmap_remove(self->_filter, key_index);
    }
    else {
        status = ALC_HASHMAP_NOTFOUND;
    }
done:
    self->status = status;
invalid_status:
    return r;
}

int hashmap_resize(hashmap_t *self, int count) {
    int status = check_valid(self);
    if(status != ALC_HASHMAP_SUCCESS) {
        goto invalid_status;
    }

    if(count > self->entries) {
        status = rehash(self, count);
        if(status != ALC_HASHMAP_SUCCESS) {
            DBG_LOG("Could not rehash to new size %d\n", count);
            status = ALC_HASHMAP_NO_MEM;
            goto done;
        }
    }
    else if(count < self->entries) {
        DBG_LOG("Requested count %d was too small.\n", count);
        status = ALC_HASHMAP_INVALID_REQ;
        goto done;
    }

done:
    self->status = status;
invalid_status:
    return status;
}

int hashmap_size(hashmap_t *self)   {
    int status = check_valid(self);
    int size = -1;
    if(status == ALC_HASHMAP_SUCCESS) {
        size = self->entries;
    }
    return size;
}

void hashmap_free(hashmap_t *self)   {
    int status = check_valid(self);
    switch(status) {
        case ALC_HASHMAP_SUCCESS:
            dynabuf_free(self->map);
            bitmap_free(self->_filter);

        case ALC_HASHMAP_INVALID:
            free(self);
        break;

        default:
            DBG_LOG("Attempted to free a NULL hashmap\n");
        break;
    }
}

int hashmap_status(hashmap_t *self)   {
    return (self == NULL) ? ALC_HASHMAP_INVALID:self->status;
}

/*
 * Helper functions
 */
static int hashmap_locate(hashmap_t *self, void *key)  {
    if(check_valid(self) != ALC_HASHMAP_SUCCESS)    {
        return -1;
    }

    uint32_t hash = self->hash(key);
    int index = hash % self->capacity;
    int start_index = index;
    bool is_valid = 0;
    bool is_equal = 0;
    while(!is_equal || !is_valid)   {
        uint8_t null_check = 0;
        
        is_valid = bitmap_contains(self->_filter, index) != 0;
        if(is_valid) {
            // dynabuf pointers are guaranteed valid
            /*
             *null_check  = *(void**)dynabuf_fetch(self->map, index) == NULL;
             *null_check  |= ((key == NULL) << 1);
             */
            is_equal = self->compare(key, *key_at(self, index)) == 0;
/*
 *            switch(null_check) {
 *                case 0: // neither is null
 *                    is_equal = self->compare(key,
 *                           *(void**)dynabuf_fetch(self->map, index)) == 0;
 *                break;
 *
 *                case 1:
 *                case 2: // exclusive null cases, wrong index.
 *                    is_equal = 0;
 *                break;
 *
 *                case 3: // both are null
 *                    is_equal = 1;
 *                break;
 *            }
 */
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

static int rehash(hashmap_t *self, int count)   {
    int status = check_valid(self);
    dynabuf_t *scratch_map;
    dynabuf_t *scratch_filter;
    if(status != ALC_HASHMAP_SUCCESS) {
        DBG_LOG("Invalid status returned from check_valid: %d\n", status);
        goto done;
    }

    if(count < self->entries || count < 1) {
        status = ALC_HASHMAP_INVALID_REQ;
        goto done;
    }

    scratch_map     = create_dynabuf(count, self->map->elem_size);
    if(scratch_map == NULL) {
        DBG_LOG("Could not create new array with size %d\n",
                self->capacity);
        status = ALC_HASHMAP_NO_MEM;
        goto done;
    }
    
    scratch_filter  = create_bitmap(count);
    if(scratch_filter == NULL) {
        DBG_LOG("Could not create new array with size %d\n",
                self->capacity);
        dynabuf_free(scratch_map);
        status = ALC_HASHMAP_NO_MEM;
        goto done;
    }

    // clear out the new buffer
    memset(scratch_map->buf, 0, count*self->map->elem_size);
    memset(scratch_filter->buf, 0, filter_size_constraint(count));

    for(int i = 0; i < self->capacity; i++)    {
        if(!bitmap_contains(self->_filter, i))   {
            continue;
        }
        uint32_t hash = self->hash(key_at(self, i));
        int index = hash % count;

        while(bitmap_contains(scratch_filter, index))  {
            index = (index + 1) % self->capacity;
        }
        dynabuf_set(scratch_map, index, dynabuf_fetch(self->map, i));
        bitmap_add(scratch_filter, index);
    }

    dynabuf_free(self->map);
    bitmap_free(self->_filter);
    self->map       = scratch_map;
    self->_filter   = scratch_filter;
    self->capacity  = count;
done:
    return status;
}

int check_valid(hashmap_t *self)    {
    int status = ALC_HASHMAP_SUCCESS;
    if(self == NULL)    {
        status = ALC_HASHMAP_FAILURE;
        goto done;
    }
    if(self->map == NULL) {
        status = ALC_HASHMAP_INVALID;
        goto done;
    }
    if(self->compare == NULL) {
        status = ALC_HASHMAP_INVALID;
        goto done;
    }
    if(self->hash == NULL)     {
        status = ALC_HASHMAP_INVALID;
        goto done;
    }
    if(self->load == NULL) {
        status = ALC_HASHMAP_INVALID;
        goto done;
    }
    if(self->_filter == NULL) {
        status = ALC_HASHMAP_INVALID;
        goto done;
    }
done:
    return status;
}

int check_space_available(hashmap_t *self, int size)  {
    int status = check_valid(self);
    if(status == ALC_HASHMAP_SUCCESS) {
        status = (self->capacity - (self->entries + size) > 0) ?
            ALC_HASHMAP_SUCCESS:ALC_HASHMAP_NO_MEM;
    }
    else {
        DBG_LOG("Invalid status returned from check_valid: %d\n", status);
    }
    return status;
}


/*
 * 75% load by default. Chosen arbitrarily.  This function is used when
 * no load function is given to the constructor.
 */
inline bool default_load(int entries, int capacity)  {
    return ((float)entries)/((float)capacity) > 0.75f;
}
