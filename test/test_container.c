#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "base.h"
#include "datetime.h"
#include "hashmap.h"
#include "list.h"
#include "log.h"
#include "rbtree.h"
#include "str.h"
#include "test.h"

// list defs
struct route_entry {
    char ip[16];
    int val;
    struct list_head list;
};
static LIST_HEAD(route_list);

struct route_entry* route_new(char* ip, int val) {
    struct route_entry* new_route;
    new_route = calloc(1, sizeof(struct route_entry));
    strcpy(new_route->ip, ip);
    new_route->val = val;
    INIT_LIST_HEAD(&new_route->list);
    return new_route;
}

struct route_entry* route_search(struct list_head* lhead, char* key) {
    struct route_entry *pos, *next;
    list_for_each_entry_safe(pos, next, lhead, list) {
        if (STR_EQUAL(pos->ip, key)) {
            // printf("%s:%d\n", pos->ip, pos->val);
            return pos;
        }
    }
    return NULL;
}

void route_entry_print(struct route_entry* entry) {
    if (entry == NULL) {
        printf("null\n");
        return;
    }
    printf("%s:%d\n", entry->ip, entry->val);
}

// rbtree defs
typedef char rbtree_key_type[16];
typedef int rbtree_val_type;

struct rbtree_entry {
    struct rb_node rb_node;
    rbtree_key_type key;
    rbtree_val_type val;
};

int rbtree_insert(struct rb_root* root, struct rbtree_entry* entry) {
    // printf("insert %s\n", entry->key);
    struct rb_node** n = &root->rb_node;
    struct rb_node* parent = NULL;
    struct rbtree_entry* e = NULL;
    while (*n) {
        parent = *n;
        e = rb_entry(*n, struct rbtree_entry, rb_node);
        if (strcmp(entry->key, e->key) < 0) {
            // if (entry->key < e->key) {
            n = &(*n)->rb_left;
            // } else if (entry->key > e->key) {
        } else if (strcmp(entry->key, e->key) > 0) {
            n = &(*n)->rb_right;
        } else {
            return -1;
        }
    }

    rb_link_node(&entry->rb_node, parent, n);
    rb_insert_color(&entry->rb_node, root);
    return 0;
}

int rbtree_remove(struct rb_root* root, struct rbtree_entry* entry) {
    printf("remove %s\n", entry->key);
    rb_erase(&entry->rb_node, root);
    return 0;
}

struct rbtree_entry* rbtree_search(struct rb_root* root, const rbtree_key_type key) {
    struct rb_node* n = root->rb_node;
    struct rbtree_entry* e = NULL;
    while (n) {
        e = rb_entry(n, struct rbtree_entry, rb_node);
        // if (*key < e->key) {
        if (strcmp(key, e->key) < 0) {
            n = n->rb_left;
            // } else if (*key > e->key) {
        } else if (strcmp(key, e->key) > 0) {
            n = n->rb_right;
        } else {
            return e;
        }
    }
    return NULL;
}

void rbtree_entry_print(struct rbtree_entry* entry) {
    if (entry == NULL) {
        printf("null\n");
        return;
    }
    printf("%s:%d\n", entry->key, entry->val);
}

void test_container() {
    unsigned long long start, end;
    char keys[10000][16] = {0};
    for (int i = 0; i < 10000; ++i) {
        sprintf(keys[i], "192.168.%d.%d", rand() % 100, rand() % 100);
    }

    // list
    struct route_entry* route_entry;
    struct route_entry route_entries[10000];
    start = gettimeofday_us();
    for (int j = 0; j < 100; ++j) {
        for (int i = 0; i < 100; ++i) {
            sprintf(route_entries[100 * j + i].ip, "192.168.%d.%d", j, i);
            route_entries[100 * j + i].val = 100 * j + i;
            INIT_LIST_HEAD(&route_entries[100 * j + i].list);
        }
    }
    end = gettimeofday_us();
    log_info("[List] init 10000 nodes need %ld kb memory", sizeof(route_entries) / 1000);
    log_info("[List] init 10000 nodes elapse time: %ld microseconds", end - start);

    start = gettimeofday_us();
    for (int j = 0; j < 100; ++j) {
        for (int i = 0; i < 100; ++i) {
            list_add(&route_entries[100 * j + i].list, &route_list);
        }
    }
    end = gettimeofday_us();
    log_info("[List] insert 10000 nodes elapse time: %ld microseconds", end - start);

    start = gettimeofday_us();
    for (int j = 0; j < 100; ++j) {
        for (int i = 0; i < 100; ++i) {
            route_entry = route_search(&route_list, keys[100 * j + i]);
        }
    }
    end = gettimeofday_us();
    log_info("[List] search 10000 times elapse time: %ld microseconds", end - start);

    // rbtree
    struct rb_root root = {NULL};
    struct rbtree_entry* entry = NULL;
    struct rbtree_entry entries[10000];
    start = gettimeofday_us();
    for (int j = 0; j < 100; ++j) {
        for (int i = 0; i < 100; ++i) {
            memset(&entries[100 * j + i], 0, sizeof(struct rbtree_entry));
            sprintf(entries[100 * j + i].key, "192.168.%d.%d", j, i);
            entries[100 * j + i].val = 100 * j + i;
        }
    }
    end = gettimeofday_us();
    log_info("[RBTree] init 10000 nodes need %ld kb memory", sizeof(entries) / 1000);
    log_info("[RBTree] init 10000 nodes elapse time: %ld microseconds", end - start);

    start = gettimeofday_us();
    for (int j = 0; j < 100; ++j) {
        for (int i = 0; i < 100; ++i) {
            rbtree_insert(&root, &entries[100 * j + i]);
        }
    }
    end = gettimeofday_us();
    log_info("[RBTree] insert 10000 nodes elapse time: %ld microseconds", end - start);

    start = gettimeofday_us();
    for (int j = 0; j < 100; ++j) {
        for (int i = 0; i < 100; ++i) {
            entry = rbtree_search(&root, keys[100 * j + i]);
            // rbtree_entry_print(entry);
        }
    }
    end = gettimeofday_us();
    log_info("[RBTree] search 10000 times elapse time: %ld microseconds", end - start);

    // hashmap
    map_int_t route_map;
    char map_key[10000][16] = {0};
    for (int j = 0; j < 100; ++j) {
        for (int i = 0; i < 100; ++i) {
            sprintf(map_key[100 * j + i], "192.168.%d.%d", j, i);
        }
    }
    log_info("[Hashmap] init 10000 nodes need %ld kb memory", (sizeof(map_int_t) + 20) * 10);
    
    start = gettimeofday_us();
    for (int j = 0; j < 100; ++j) {
        for (int i = 0; i < 100; ++i) {
            sprintf(map_key[100 * j + i], "192.168.%d.%d", j, i);
            map_set(&route_map, map_key[j * 100 + i], j * 100 + i);
        }
    }
    end = gettimeofday_us();
    log_info("[Hashmap] insert 10000 nodes elapse time: %ld microseconds", end - start);

    start = gettimeofday_us();
    for (int j = 0; j < 100; ++j) {
        for (int i = 0; i < 100; ++i) {
            map_get(&route_map, keys[j * 100 + i]);
        }
    }
    end = gettimeofday_us();
    log_info("[Hashmap] search 10000 times elapse time: %ld microseconds", end - start);
}
