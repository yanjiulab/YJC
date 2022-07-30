#include <stdio.h>

#include "hashmap.h"
#include "test.h"
void test_map() {
    map(int) *m = map_new(int);

    map_set(m, "a", 1);
    map_set(m, "b", 11);
    map_set(m, "c", 12);
    map_set(m, "d", 13);
    map_set(m, "d", "abc");
    const char *key;
    map_iter_t iter = map_iter(m);

    while ((key = map_next(m, &iter))) {
        printf("%s: %d\n", key, *map_get(m, key));
    }
}