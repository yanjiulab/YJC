#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"
#include "log.h"
#include "ptable.h"
#include "test.h"

// 定义节点结构体
struct fox {
    unsigned int tail_len;
    unsigned int weight;
    char color[16];
    struct list_head list;
};

// 定义链表头节点
static LIST_HEAD(fox_list);

// 动态创建节点
struct fox* fox_new(unsigned int len, unsigned int weight, char* col) {
    struct fox* new_fox;
    new_fox = calloc(1, sizeof(struct fox));
    new_fox->tail_len = len;
    new_fox->weight = weight;
    strcpy(new_fox->color, col);
    INIT_LIST_HEAD(&new_fox->list);

    return new_fox;
}

// 静态创建节点
// struct fox red_fox = {.tail_len = 40, .weight = 6, .list=LIST_HEAD_INIT(red_fox.list)}

// 打印节点
void fox_show(struct fox* f) { printf("Fox{tail_len=%d,weight=%d,color=%s}\n", f->tail_len, f->weight, f->color); }

// 打印链表
void fox_list_show(struct list_head* fox_list) {
    FILE* fp = fopen("log/app.log", "a");
    struct fox* f;
    struct ptable t;
    // ptable_t t = ptable_new();
    ptable_init(&t, "TailLen", "%d", "Weight", "%d", "Color", "%s", NULL);
    list_foreach(f, fox_list, list) { ptable_add(&t, f->tail_len, f->weight, f->color); }
    ptable_print(&t, 60, stdout);
    ptable_free(&t);
    fclose(fp);
}

void test_list() {
    FILE* fp = fopen("log/app.log", "w");
    // int file_level = LOG_DEBUG;
    // log_add_fp(fp, file_level);

    struct list_head* foxs = list_new();

    // 添加节点
    struct fox* fox;
    fox = fox_new(30, 10, "red");
    list_add(&fox->list, foxs);
    fox_list_show(foxs);
    fox = fox_new(40, 8, "brown");
    list_add(&fox->list, foxs);
    fox = fox_new(20, 7, "yellow");
    list_add_tail(&fox->list, foxs);
    fox_list_show(foxs);

    // 遍历节点
    struct fox* f;
    list_foreach(f, foxs, list) { fox_show(f); }
    list_foreach_reverse(f, foxs, list) { fox_show(f); }

    // 安全遍历节点（遍历时删除）

    struct fox *pos, *next;
    list_foreach_safe(pos, next, foxs, list) {
        if (pos->weight == 8) list_del(&pos->list);
    }
    fox_list_show(foxs);

    struct fox *pos1, *next1;
    list_foreach_safe_reverse(pos1, next1, foxs, list) {
        fox_show(pos1);
        if (pos1->tail_len == 20) list_del(&pos1->list);
    }
    fox_list_show(foxs);

    printf("list_empty? %s\n", list_empty(foxs) ? "empty" : "not empty");
    printf("list_is_singular? %s\n", list_is_singular(foxs) ? "singular" : "not singular");

    free(fox);
    free(foxs);
}