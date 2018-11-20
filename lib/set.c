#include <alibc/extensions/set.h>
#include <alibc/extensions/dynabuf.h>
#include <alibc/extensions/debug.h>
#include <stdlib.h>
#include <string.h>

// private functions
static int set_resize(set_t *self, int count);
static int check_valid(set_t *self);
static int check_space_available(set_t *self, int size);
static int set_locate(set_t *self, void *item);

set_t *create_set(int size, hash_type *hashfn, cmp_type *comparefn) {
    set_t *r    = malloc(sizeof(set_t));
    if(r == NULL) {
        DBG_LOG("Could not malloc set container\n");
        r = NULL;
        goto finish;
    }
    
    r->buf      = create_dynabuf(size*sizeof(void*));
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
    r->compare  = comparefn;
    r->status   = SET_SUCCESS;
finish:
    return r;
}

static int set_resize(set_t *self, int count) {
    int status = SET_INVALID;
    if(check_valid(self) != SET_SUCCESS) {
        goto finish;
    }

    dynabuf_t   *scratch_buf;
    dynabuf_t   *scratch_filter;

    scratch_buf     = create_dynabuf(sizeof(void*)*count);
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
        
        uint32_t hash   = self->hash(self->buf->buf[i]);
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
        scratch_buf->buf[index] = self->buf->buf[i];
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
            if(set_resize(self, (self->capacity * 2) + 1) == SET_SUCCESS) {
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
        index = (index + 1) % self->capacity;
        if(index == start_index) {
            if(set_resize(self, (self->capacity * 2) + 1) == SET_SUCCESS) {
                status = set_add(self, item);
                goto finish;
            }
            else {
                DBG_LOG("could not resize set on add, but space was reported "
                        "available, structure corruption is likely.\n");
                self->status = SET_NO_MEM;
                status = SET_NO_MEM;
                goto finish;
            }
        }
    }
    
    self->buf->buf[index] = item;
    bitmap_add(self->_filter, index);
    self->entries++;

    self->status = SET_SUCCESS;
    status = SET_SUCCESS;
finish:
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
        r = self->buf->buf[index];
    }
    else {
        self->status = SET_NOTFOUND;
        r = NULL;
    }
finish:
    return r;
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
        is_valid = bitmap_contains(self->_filter, index) != 0;
        if(self->buf->buf[index] == NULL) {
            if(item == NULL) {
                return index;
            }
            else {
                is_equal = 0;
            }
        }
        else {
            is_equal = self->compare(item, self->buf->buf[index]) == 0;
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
