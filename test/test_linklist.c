#include "linklist.h"
#include "test.h"

bool int_iter(const void* item, void* udata) {
    int i = (int)item;
    printf("%d\n", i);
    return true;
}

void test_linklist() {
    struct list* list;
    list = list_create(NULL, NULL);
    for (size_t i = 1; i < 10; i++) {
        list_add(list, i);
    }
    list_scan(list, int_iter, 0);

    int* i;
    struct listnode *node, *nnode;
    list_foreach(list, node, nnode, i) {
        printf("%d\n", i);
    }
    
    list_foreach(list, node, nnode, i) {
        printf("%d\n", i);
    }

    list_free(&list);
}
