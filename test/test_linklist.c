#include "linklist.h"
#include "test.h"

#include <string.h>

bool int_iter(const void* item, void* udata) {
    int i = (int)item;
    printf("%d\n", i);
    return true;
}

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

void test_linklist() {
    struct list* list;
    struct listnode *node, *nnode;

    list = list_create(NULL, NULL);
    for (size_t i = 1; i < 10; i++) {
        list_add(list, i);
    }
    list_scan(list, int_iter, 0);

    int* i;
    list_foreach(list, node, nnode, i) {
        printf("%d\n", i);
    }
    list_free(&list);

    list = list_create(NULL, NULL);
    // struct user user1 = {.name = "Dale", .age = 44};
    // struct user user2 = {.name = "Roger", .age = 68};
    // struct user user3 = {.name = "Jane", .age = 47};

    list_add(list, user_new("Dale", 44));
    list_add(list, user_new("Roger", 46));
    list_add(list, user_new("Jane", 33));

    // list_del(list, user_new("Dale", 33));
    struct user* user;
    list_foreach(list, node, nnode, user) {
        user_iter(user, NULL);
    }

    user = list_get(list, USER_KEY("Dale"));
    if (user)
        user_iter(user, NULL);
    else
        printf("item not found\n");

    list_del(list, USER_KEY("Dale"));
    
    list_scan(list, user_iter, 0);

    list_free(&list);
}
