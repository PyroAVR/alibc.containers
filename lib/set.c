#include <alibc/extensions/set.h>
#include <alibc/extensions/dynabuf.h>
#include <alibc/extensions/debug.h>
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
    set_t *r    = malloc(sizeof(set_t));
    if(r == NULL) {
        DBG_LOG("Could not malloc set container\n");
        r = NULL;
        goto finish;
    }
    
    r->buf      = create_dynabuf(size, unit);
    if(r->buf == NULL) {
        DBG_LOG("Could not malloc backing buffer for set\n");
        free(r);
        r = NULL;
        goto finish;
    }

    r->_filter  = create_bitmap(size);
    if(r->_filter == NULL) {
        DBG_LOG("Could not malloc filter bitmap for set \n");
        dynabuf_free(r->buf);
        free(r);
        r = NULL;
        goto finish;
    }

    r->entries  = 0;
    r->capacity = size;
    r->hash     = hashfn;

    if(loadfn == NULL) {
        r->load     = default_load;
    }
    else {
        r->load     = loadfn;
    }

    r->compare  = comparefn;
    r->status   = SET_SUCCESS;
finish:
    return r;
}

static int rehash(set_t *self, int count) {
    int status = SET_INVALID;
    if(check_valid(self) != SET_SUCCESS) {
        goto finish;
    }

    dynabuf_t   *scratch_buf;
    dynabuf_t   *scratch_filter;

    scratch_buf     = create_dynabuf(count, self->buf->elem_size);
    if(scratch_buf == NULL) {
        DBG_LOG("Could not create new backing array with size %d\n",
                self->capacity);
        status = SET_NO_MEM;
        goto finish;
    }
    scratch_filter  = create_bitmap(count);
    if(scratch_filter == NULL) {
        DBG_LOG("Could not create new bitmap with size %d\n",
                self->capacity);
        bitmap_free(scratch_filter);
        status = SET_NO_MEM;
        goto finish;
    }

    memset(scratch_buf->buf, 0, sizeof(void*)*count);
    memset(scratch_filter->buf, 0, count >> 3);

    for(uint32_t i = 0; i < self->capacity; i++) {
        if(!bitmap_contains(self->_filter, i)) {
            continue;
        }
        
        uint32_t hash   = self->hash(dynabuf_fetch(self->buf,i));
        uint32_t index  = hash % count;
        uint32_t start_index = index;
        while(bitmap_contains(scratch_filter, index)) {
            index = (index + 1) % self->capacity;
            if(index == start_index) {
                DBG_LOG("Could not find a suitable location to insert hash: "
                        "%d and new size: %d\n", hash, count);
                dynabuf_free(scratch_buf);
                bitmap_free(scratch_filter);
                self->status = SET_NO_MEM;
                status = SET_NO_MEM;
                goto finish;
            }
        }
        dynabuf_set(scratch_buf, index, dynabuf_fetch(self->buf, i));
        /*
         *scratch_buf->buf[index] = self->buf->buf[i];
         */
        bitmap_add(scratch_filter, index);
    }

    dynabuf_free(self->buf);
    bitmap_free(self->_filter);
    self->buf       = scratch_buf;
    self->_filter   = scratch_filter;
    self->capacity  = count;
    self->status    = SET_SUCCESS;
    status = SUCCESS;
finish:
    return status;
}

int set_add(set_t *self, void *item) {
    int status = SET_INVALID; 
    switch(check_space_available(self, 1)) {
        case SET_INVALID:
            goto finish;
        break;

        case SET_NO_MEM:
            DBG_LOG("Set was resized for item %p\n", item);
            if(rehash(self, (self->capacity * 2) + 1) == SET_SUCCESS) {
                status = set_add(self, item);
                goto finish;
            }
            else {
                DBG_LOG("could not resize set on add, no mem available.\n");
                self->status = SET_NO_MEM;
                status = SET_NO_MEM;
                goto finish;
            }
        break;

        default:
        break;
    }

    uint32_t    hash        = self->hash(item);
    uint32_t    index       = hash % self->capacity;
    uint32_t    start_index = index;

    // loop until we find an open spot to insert into
    while(bitmap_contains(self->_filter, index)) {
        if(dynabuf_fetch(self->buf, index) != NULL &&
                self->compare(item, dynabuf_fetch(self->buf, index)) == 0) {

            DBG_LOG("got repeat item case\n");
            goto repeat_item;
        }
        index = (index + 1) % self->capacity;
        if(index == start_index) {
            if(rehash(self, (self->capacity * 2) + 1) == SET_SUCCESS) {
                status = set_add(self, item);
                goto finish;
            }
            else {
                DBG_LOG("could not resize set on add, but space was reported "
                        "available, structure corruption is likely.\n");
                status = SET_NO_MEM;
                goto finish;
            }
        }
    }
    
    self->entries++;
    bitmap_add(self->_filter, index);
repeat_item:
    dynabuf_set(self->buf, index, item);
    /*
     *self->buf->buf[index] = item;
     */

    status = SET_SUCCESS;
    if(self->load(self->entries, self->capacity) == true) {
        status = set_resize(self, 2*self->capacity + 1);
    }
finish:
    self->status = status;
    return status;
}


void *set_remove(set_t *self, void *item) {
    void *r = NULL;
    if(check_valid(self) != SET_SUCCESS) {
        goto finish;
    }

    int index = set_locate(self, item);

    if(index != -1) {
        bitmap_remove(self->_filter, index);
        self->entries--;
        self->status = SET_SUCCESS;
        r = dynabuf_fetch(self->buf, index);
    }
    else {
        self->status = SET_NOTFOUND;
        r = NULL;
    }
finish:
    return r;
}

int set_resize(set_t *self, int count) {
    int status = SET_SUCCESS;
    if(check_valid(self) != SET_SUCCESS) {
        status = SET_INVALID;
        goto finish_nostat;
    }

    if(count > self->entries) {
        status = rehash(self, count);
        if(status != SET_SUCCESS) {
            DBG_LOG("Could not rehash to new size %d\n", count);
            status = SET_NO_MEM;
            goto finish;

        }
    }
    else if(count < self->entries) {
        DBG_LOG("Requested count %d was too small.\n", count);
        status = SET_INVALID_REQ;
        goto finish;
    }

finish:
    self->status = status;
finish_nostat:
    return status;
}
int set_contains(set_t *self, void *item) {
    int r = 0;
    if(check_valid(self) != SET_SUCCESS) {
        goto finish;
    }

    int index = set_locate(self, item);
    if(index != -1) {
        self->status = SET_SUCCESS;
        r = 1;
    }
    else {
        self->status = SET_NOTFOUND;
        r = 0;
    }
finish:
    return r;
}


int set_size(set_t *self){
    int r = -1;
    if(check_valid(self) != SET_SUCCESS) {
        goto finish;
    }
    r = self->entries;
finish:
    return r;
}


/*
 *void *set_iterate(set_t *self, iter_context *context) {
 *    return 0;
 *}
 */

int set_okay(set_t *self) {
    int status = SET_SUCCESS;
    if((status = check_valid(self)) == SET_SUCCESS) {
        status = self->status;
    }
    return status;
}

void set_free(set_t *self) {
    if(self == NULL) {
        return;
    }
    
    if(self->buf != NULL) {
        dynabuf_free(self->buf);
    }
    
    if(self->_filter != NULL) {
        bitmap_free(self->_filter);
    }

    free(self);
}

/*
 * helper functions
 */

static int check_valid(set_t *self) {
    int r = SET_INVALID;
    if(self == NULL) {
        goto finish;
    }

    if(self->buf == NULL) {
        r = SET_INVALID;
        goto finish;
    }

    if(self->_filter == NULL) {
        r = SET_INVALID;
        goto finish;
    }

    if(self->compare == NULL) {
        r = SET_INVALID;
        goto finish;
    }

    if(self->load == NULL) {
        r = SET_INVALID;
        goto finish;
    }

    r = SET_SUCCESS;
finish:
    return r;
}


int check_space_available(set_t *self, int size)  {
    int status = SET_NO_MEM;
    switch((status = check_valid(self))) {
        case SET_SUCCESS:
            status = ((self->entries + size) <= self->capacity) ? 
                    SET_SUCCESS:SET_NO_MEM;
        break;
        default:
            DBG_LOG("Invalid status returned from check_valid: %d\n", status);
    }
    return status;
}

static int set_locate(set_t *self, void *item) {
    uint32_t    hash        = self->hash(item);
    uint32_t    index       = hash % self->capacity;
    uint32_t    start_index = index;
    uint8_t     is_valid    = 0;
    uint8_t     is_equal    = 0;
    while(!is_equal || !is_valid) {
        uint8_t null_check = 0;

        is_valid = bitmap_contains(self->_filter, index) != 0;
        if(is_valid) {
            null_check  = dynabuf_fetch(self->buf, index) == NULL;
            null_check  |= ((item == NULL) << 1);
            switch(null_check) {
                case 0:
                    is_equal = self->compare(
                            item,
                            dynabuf_fetch(self->buf, index)
                    ) == 0;
                break;

                case 1:
                case 2:
                    is_equal = 0; 
                break;

                case 3:
                    is_equal = 1;
                break;
            }
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
