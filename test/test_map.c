#include <stdio.h>

#include "map.h"
#include "test.h"
void test_map() {
    map(int) *m = map_new(int);

    map_insert(m, "a", 1);
    map_insert(m, "b", 11);
    map_insert(m, "c", 12);
    map_insert(m, "d", 13);
    map_insert(m, "saf", 0);
    map_insert(m, "", 1);
    map_insert(m, "", 100);

    const char *key;
    map_iter_t iter = map_iter(m);
    while ((key = map_next(m, &iter))) {
        printf("%s: %d\n", key, *map_get(m, key));
    }

    // for (map_iter_t iter = map_iter(m); key = map_next(m, &iter);) {
    //     printf("%s: %d\n", key, *map_get(m, key));
    // }

    // const char *;
    int val;
    // map_foreach(m, key, val) {
    //     printf("%s: %d\n", key, val);
    // }

    map_foreach(m, key, val) {
        printf("%s: %d\n", key, val);
        // val = *map_get(m, key);
    }

    printf("map size: %d, cap: %d, empty?: %d\n", map_size(m), map_capacity(m), map_empty(m));
    map_dump(m, "%d");
}