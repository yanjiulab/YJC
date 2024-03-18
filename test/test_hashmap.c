#include "hashmap.h"
#include "test.h"

#include <stdio.h>
#include <string.h>

struct user {
    char* name;
    int age;
    double weight;
};

#define USER_ENTRY(n, a, w) (&(struct user){.name = n, .age = a, .weight = w})
#define USER_KEY(n, a)      (&(struct user){.name = n, .age = a})

int user_compare(const void* a, const void* b, void* udata) {
    const struct user* ua = a;
    const struct user* ub = b;
    int ret;
    if (ret = (strcmp(ua->name, ub->name)))
        return ret;
    return ua->age - ub->age;
}

bool user_iter(const void* item, void* udata) {
    const struct user* user = item;
    printf("%s (age=%d, weight=%f)\n", user->name, user->age, user->weight);
    return true;
}

uint64_t user_hash(const void* item, uint64_t seed0, uint64_t seed1) {
    const struct user* user = item;
    uint64_t key;

    key = hashmap_sip(user->name, strlen(user->name), seed0, seed1);
    return hashmap_xxhash3(&user->age, sizeof(int), key, key);
}

void test_hashmap() {
    // create a new hash map where each item is a `struct user`. The second
    // argument is the initial capacity. The third and fourth arguments are
    // optional seeds that are passed to the following hash function.
    struct hashmap* map = hashmap_new(sizeof(struct user), 0, 0, 0,
                                      user_hash, user_compare, NULL, NULL);

    // Here we'll load some users into the hash map. Each set operation
    // performs a copy of the data that is pointed to in the second argument.
    hashmap_set(map, &(struct user){.name = "Jane", .age = 47, .weight = 50.31});
    hashmap_set(map, USER_ENTRY("Dale", 44, 65.39));
    hashmap_set(map, USER_ENTRY("Roger", 68, 78.55));

    struct user* user;
    printf("\n-- get some users --\n");

    user = hashmap_get(map, USER_KEY("Jane", 47));
    printf("%s (age=%d, weight=%f)\n", user->name, user->age, user->weight);

    user = hashmap_get(map, USER_KEY("Roger", 68));
    printf("%s (age=%d, weight=%f)\n", user->name, user->age, user->weight);

    user = hashmap_get(map, USER_KEY("Dale", 44));
    printf("%s (age=%d, weight=%f)\n", user->name, user->age, user->weight);

    user = hashmap_get(map, &(struct user){.name = "Jane", .age = 18});
    printf("%s\n", user ? "exists" : "not exists");

    printf("\n-- iterate over all users (hashmap_scan) --\n");
    hashmap_scan(map, user_iter, NULL);

    printf("\n-- iterate over all users (hashmap_iter) --\n");
    size_t iter = 0;
    void* item;
    while (hashmap_iter(map, &iter, &item)) {
        const struct user* user = item;
        printf("%s (age=%d, weight=%f)\n", user->name, user->age, user->weight);
    }

    printf("\n-- delete --\n");
    user = hashmap_del(map, USER_KEY("Dale", 44));
    printf("%s (age=%d, weight=%f) delete\n", user->name, user->age, user->weight);

    printf("\n-- delete --\n");
    user = hashmap_set(map, USER_ENTRY("Roger", 68, 69.23));
    if (user)
        printf("%s (age=%d, weight=%f) update\n", user->name, user->age, user->weight);

    user = hashmap_set(map, USER_ENTRY("Roger", 55, 72));

    printf("\n-- iterate over all users (hashmap_scan) --\n");
    hashmap_scan(map, user_iter, NULL);

    hashmap_free(map);
}
