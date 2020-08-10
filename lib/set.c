#include <alibc/containers/set.h>
#include <alibc/containers/dynabuf.h>
#include <alibc/containers/debug.h>
#include <stdlib.h>
#include <string.h>

// private functions
static int rehash(set_t *self, int count);
static int check_valid(set_t *self);
static int check_space_available(set_t *self, int size);
static int set_locate(set_t *self, void *item);
static inline bool default_load(int, int);

set_t *create_set(int size, int unit, hash_type *hashfn,
        cmp_type *comparefn, load_type *loadfn) {
    set_t *r = malloc(sizeof(set_t));
    if(r == NULL) {
        DBG_LOG("Could not malloc set container\n");
        r = NULL;
        goto done;
    }
    
    r->buf = create_dynabuf(size, unit);
    if(r->buf == NULL) {
        DBG_LOG("Could not malloc backing buffer for set\n");
        free(r);
        r = NULL;
        goto done;
    }

    r->_filter = create_bitmap(size);
    if(r->_filter == NULL) {
        DBG_LOG("Could not malloc filter bitmap for set \n");
        dynabuf_free(r->buf);
        free(r);
        r = NULL;
        goto done;
    }

    r->entries = 0;
    r->capacity = size;
    r->hash = hashfn;

    if(loadfn == NULL) {
        r->load = default_load;
    }
    else {
        r->load = loadfn;
    }

    r->compare = comparefn;
    r->status = ALC_SET_SUCCESS;
done:
    return r;
}

static int rehash(set_t *self, int count) {
    int status = ALC_SET_INVALID;
    if(check_valid(self) != ALC_SET_SUCCESS) {
        goto done;
    }

    if(count < self->entries || count < 1) {
        status = ALC_SET_INVALID_REQ;
        goto done;
    }

    dynabuf_t *scratch_buf;
    dynabuf_t *scratch_filter;

    scratch_buf = create_dynabuf(count, self->buf->elem_size);
    if(scratch_buf == NULL) {
        DBG_LOG("Could not create new backing array with size %d\n",
                self->capacity);
        status = ALC_SET_NO_MEM;
        goto done;
    }
    scratch_filter = create_bitmap(count);
    if(scratch_filter == NULL) {
        DBG_LOG("Could not create new bitmap with size %d\n",
                self->capacity);
        bitmap_free(scratch_filter);
        status = ALC_SET_NO_MEM;
        goto done;
    }

    memset(scratch_buf->buf, 0, sizeof(void*)*count);
    memset(scratch_filter->buf, 0, count >> 3);

    for(int i = 0; i < self->capacity; i++) {
        if(!bitmap_contains(self->_filter, i)) {
            continue;
        }
        
        void **temp_item = dynabuf_fetch(self->buf,i);
        uint32_t hash = self->hash(*temp_item);
        int index = hash % count;

        while(bitmap_contains(scratch_filter, index)) {
            index = (index + 1) % self->capacity;
        }
        dynabuf_set(scratch_buf, index, *temp_item);
        bitmap_add(scratch_filter, index);
    }

    dynabuf_free(self->buf);
    bitmap_free(self->_filter);
    self->buf = scratch_buf;
    self->_filter = scratch_filter;
    self->capacity = count;
    self->status = ALC_SET_SUCCESS;
    status = ALC_SET_SUCCESS;
done:
    return status;
}

int set_add(set_t *self, void *item) {
    int status = ALC_SET_INVALID; 
    switch(check_space_available(self, 1)) {
        case ALC_SET_INVALID:
            goto invalid_status;
        break;

        case ALC_SET_NO_MEM:
            DBG_LOG("Set was resized for item %p\n", item);
            if(rehash(self, (self->capacity * 2) + 1) == ALC_SET_SUCCESS) {
                status = set_add(self, item);
                goto done;
            }
            else {
                DBG_LOG("could not resize set on add, no mem available.\n");
                self->status = ALC_SET_NO_MEM;
                status = ALC_SET_NO_MEM;
                goto done;
            }
        break;

        default:
        break;
    }

    uint32_t hash = self->hash(item);
    int index = hash % self->capacity;

    // loop until we find an open spot to insert into
    while(bitmap_contains(self->_filter, index)) {
        void **temp_item = dynabuf_fetch(self->buf, index);
        if(temp_item != NULL && *temp_item != NULL &&
                self->compare(item, *temp_item) == 0) {

            DBG_LOG("got repeat item case\n");
            goto repeat_item;
        }
        index = (index + 1) % self->capacity;
    }
    
    self->entries++;
    bitmap_add(self->_filter, index);
repeat_item:
    dynabuf_set(self->buf, index, item);

    status = ALC_SET_SUCCESS;
    if(self->load(self->entries, self->capacity) == true) {
        status = set_resize(self, 2*self->capacity + 1);
    }
done:
    self->status = status;
invalid_status:
    return status;
}


void **set_remove(set_t *self, void *item) {
    void **r = NULL;
    if(check_valid(self) != ALC_SET_SUCCESS) {
        goto done;
    }

    int index = set_locate(self, item);

    if(index != -1) {
        bitmap_remove(self->_filter, index);
        self->entries--;
        self->status = ALC_SET_SUCCESS;
        r = dynabuf_fetch(self->buf, index);
    }
    else {
        self->status = ALC_SET_NOTFOUND;
        r = NULL;
    }
done:
    return r;
}

int set_resize(set_t *self, int count) {
    int status = ALC_SET_SUCCESS;
    if(check_valid(self) != ALC_SET_SUCCESS) {
        status = ALC_SET_INVALID;
        goto invalid_status;
    }

    if(count > self->entries) {
        status = rehash(self, count);
        if(status != ALC_SET_SUCCESS) {
            DBG_LOG("Could not rehash to new size %d\n", count);
            status = ALC_SET_NO_MEM;
            goto done;

        }
    }
    else if(count < self->entries) {
        DBG_LOG("Requested count %d was too small.\n", count);
        status = ALC_SET_INVALID_REQ;
        goto done;
    }

done:
    self->status = status;
invalid_status:
    return status;
}


int set_contains(set_t *self, void *item) {
    int r = 0;
    if(check_valid(self) != ALC_SET_SUCCESS) {
        goto done;
    }

    int index = set_locate(self, item);
    if(index != -1) {
        self->status = ALC_SET_SUCCESS;
        r = 1;
    }
    else {
        self->status = ALC_SET_NOTFOUND;
        r = 0;
    }
done:
    return r;
}


int set_size(set_t *self){
    int r = -1;
    if(check_valid(self) != ALC_SET_SUCCESS) {
        goto done;
    }
    r = self->entries;
done:
    return r;
}


int set_status(set_t *self) {
    int status = ALC_SET_SUCCESS;
    if((status = check_valid(self)) == ALC_SET_SUCCESS) {
        status = self->status;
    }
    return status;
}

void set_free(set_t *self) {
    if(self == NULL) {
        goto done;
    }
    
    if(self->buf != NULL) {
        dynabuf_free(self->buf);
    }
    
    if(self->_filter != NULL) {
        bitmap_free(self->_filter);
    }

    free(self);
done:
    return;
}

/*
 * helper functions
 */

static int check_valid(set_t *self) {
    int r = ALC_SET_INVALID;
    if(self == NULL) {
        goto done;
    }

    if(self->buf == NULL) {
        r = ALC_SET_INVALID;
        goto done;
    }

    if(self->_filter == NULL) {
        r = ALC_SET_INVALID;
        goto done;
    }

    if(self->compare == NULL) {
        r = ALC_SET_INVALID;
        goto done;
    }

    if(self->load == NULL) {
        r = ALC_SET_INVALID;
        goto done;
    }

    r = ALC_SET_SUCCESS;
done:
    return r;
}


int check_space_available(set_t *self, int size)  {
    int status = ALC_SET_NO_MEM;
    switch((status = check_valid(self))) {
        case ALC_SET_SUCCESS:
            status = (self->capacity - (self->entries + size) > 0) ? 
                    ALC_SET_SUCCESS:ALC_SET_NO_MEM;
        break;
        default:
            DBG_LOG("Invalid status returned from check_valid: %d\n", status);
    }
    return status;
}

static int set_locate(set_t *self, void *item) {
    uint32_t hash = self->hash(item);
    int index = hash % self->capacity;
    int start_index = index;
    bool     is_valid    = 0;
    bool     is_equal    = 0;
    while(!is_equal || !is_valid) {
        uint8_t null_check = 0;

        is_valid = bitmap_contains(self->_filter, index) != 0;
        if(is_valid) {
            void **temp_item = dynabuf_fetch(self->buf, index);
            // determine if both items are null, or just one, or zero.
            /*null_check  =  *temp_item == NULL;*/
            /*null_check  |= ((item == NULL) << 1);*/
            /*
             *switch(null_check) {
             *    case 0:
             */
                    is_equal = self->compare(item, *temp_item) == 0;
/*
 *                break;
 *
 *                case 1:
 *                case 2:
 *                    is_equal = 0; 
 *                break;
 *
 *                case 3:
 *                    is_equal = 1;
 *                break;
 *            }
 */
        }

        if(is_valid && is_equal) {
            break;
        }

        index = (index + 1) % self->capacity;
        if(index == start_index) {
            index = -1;
            break;
        }
    }
    return index;
}

/*
 * 75% load by default. Chosen arbitrarily
 */
inline bool default_load(int entries, int capacity)  {
    return ((float)entries)/((float)capacity) > 0.75f;
}
