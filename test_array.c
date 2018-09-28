#include <stdio.h>
#include "include/list.h"
#include "include/array.h"
// ideas: 
// implementation enforcement with macros via IMPLEMENTS(proto)
// inheritance via extension, possibly after the { on a struct
// # define protocol(...) typedef struct {???

/*
 *struct test {
 *    proto_list;
 *    int x;
 *};
 */

const char *data[] = {"the quick", "brown fox", "jumped over", "the lazy dog"};

int main(void)  {
    proto_list *array = create_array(2);
    for(int i = 0; i < 4; i++)   {
        printf("%s\n", data[i]);
        array->append(array, (void*)data[i]);
    }
    array->swap(array, 0, 3);
    printf("removed: %s\n", array->remove(array, 1));
    array->insert(array, 0, "lorem ipsum");
    for(int i = 0; i < array->size(array); i++) {
        printf("%s\n", (char*)array->fetch(array, i));
    }
    array->free(array);
    return 0;
}
