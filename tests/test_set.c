#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <alibc/extensions/set.h>
#include <alibc/extensions/hashmap.h>
#include <criterion/criterion.h>
#include <string.h>

set_t *set_uut;

char *items[] = {"do not go gentle into that good night,",
                 "old age should burn and rave at the close of day,",
                 "rage, rage, against the dying of the light.",
                 "though wise men at their end know dark is right,",
                 "because their words had forked no lightning they",
                 "do not go gentle into that good night."
};

void set_init(void) {
    set_uut = create_set(4, hashmap_hash_str, strcmp);
}

void set_finish(void) {
    set_free(set_uut);
}

TestSuite(set_tests, .init=set_init, .fini=set_finish, .timeout=0);

Test(set_tests, add) {
    for(int i = 0; i < 6; i++)  {
        set_add(set_uut, items[i]);
    }
    for(int i = 0; i < 6; i++) {
        cr_assert(set_contains(set_uut, items[i]),
                "could not find %s in set", items[i]);
    }

    uint64_t result = set_contains(set_uut, "we live, as we dream... alone.");
    cr_assert_eq(result, 0, "found an item which was not added to the set.");
    cr_assert_eq(set_okay(set_uut), SET_NOTFOUND);

}

Test(set_tests, remove) {
    for(int i = 0; i < 6; i++)  {
        set_add(set_uut, items[i]);
    }
    for(int i = 0; i < 6; i++) {
        set_remove(set_uut, items[i]);
        cr_assert(!set_contains(set_uut, items[i]),
                "item %s found, should be missing.", items[i]);
    }

    cr_assert_eq(set_remove(set_uut, "whoops please edit"), NULL,
            "allowed removal of non-existant item");
}

Test(set_tests, resize) {
    int result;
    char *more_items[] = {"one", "two", "three", "four", "five"};
    for(int i = 0; i < 5; i++) {
        set_add(set_uut, (void*)more_items[i]);
    }
    result = set_resize(set_uut, 3);
    cr_assert_eq(result, set_okay(set_uut), "status not set");
    cr_assert_eq(result, SET_INVALID_REQ, "Allowed resize of < count of elements.");

    result = set_resize(set_uut, set_size(set_uut));
    cr_assert_eq(result, SET_SUCCESS, "resize no-op failed.");

    result = set_resize(set_uut, 50);
    cr_assert_eq(result, SET_SUCCESS, "could not resize set to size 50.");
}
/*
 *Test(set_tests, iterate) {
 *    //how on earth do I test this?
 *    for(int i = 0; i < 6; i++)  {
 *        set_add(set_uut, items[i]);
 *    }
 *    iter_context context;
 *    // iterate one too far - check stop
 *    for(int i = 0; i < 7; i++) {
 *        char *next = set_iterate(set_uut, &context);
 *        if(i == 0) {
 *            cr_assert_eq(context.index, 1, "context index was not updated");
 *        }
 *        else if(i < 6) {
 *            cr_assert_eq(context.status, ITER_CONTINUE);
 *        }
 *        else if(i == 6) {
 *            cr_assert_eq(context.status, ITER_STOP);
 *        }
 *        else if(i == 7) {
 *            // oob case
 *            cr_assert_null(next, "returned a non-null result out of bounds");
 *        }
 *        else {
 *            cr_assert(set_contains(set_uut, next));
 *        }
 *    }
 *}
 */
