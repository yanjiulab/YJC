#include "base.h"
#include "skiplist.h"
#include "test.h"
#include <stdbool.h>

#define USER_KEY(val) (&(struct user){.name = val})

static struct user {
    char* name;
    int age;
};

static int user_compare(const void* a, const void* b, void* udata) {
    const struct user* ua = a;
    const struct user* ub = b;
    return strcmp(ua->name, ub->name);
}

static bool user_iter(const void* item, void* udata) {
    const struct user* user = item;
    printf("%s (age=%d)\n", user->name, user->age);
    return true;
}

static struct user* user_new(char* name, int age) {
    struct user* u = calloc(1, sizeof(struct user));
    u->name = strdup(name);
    u->age = age;
    return u;
}

void test_skiplist() {
    struct skiplist* sl = skiplist_new(0, user_compare, NULL);
    struct user* user1 = user_new("Tom", 44);

    skiplist_insert(sl, user1, user1);

    for (size_t i = 0; i < 100; i++) {
        skiplist_insert(sl, user_new(rand_str(NULL, 5), rand_int(0, 100)), rand_int(100, 101));
    }

    // skiplist_search(sl, USER_KEY("Tom"), &res);
    printf("count %d\n", skiplist_count(sl));

    skiplist_free(sl);
}
