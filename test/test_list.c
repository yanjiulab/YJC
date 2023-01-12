#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"
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
    struct fox* f;
    printf(">>>>>>>>>>>>>>\n");
    list_for_each_entry(f, fox_list, list) { fox_show(f); }
    printf("<<<<<<<<<<<<<<\n");
}

void test_list() {
    // 添加节点
    struct fox* fox;
    fox = fox_new(30, 10, "red");
    list_add(&fox->list, &fox_list);
    fox = fox_new(40, 8, "brown");
    list_add(&fox->list, &fox_list);
    fox = fox_new(20, 7, "yellow");
    list_add_tail(&fox->list, &fox_list);

    // 遍历节点
    struct fox* f;
    list_for_each_entry(f, &fox_list, list) { fox_show(f); }
    list_for_each_entry_reverse(f, &fox_list, list) { fox_show(f); }

    // 安全遍历节点（遍历时删除）
    struct fox *pos, *next;
    list_for_each_entry_safe(pos, next, &fox_list, list) {
        if (pos->weight == 8) list_del(&pos->list);
    }
    fox_list_show(&fox_list);
    list_for_each_entry_safe_reverse(pos, next, &fox_list, list) {
        fox_show(pos);
        if (pos->tail_len == 20) list_del(&pos->list);
    }
    fox_list_show(&fox_list);

    printf("list_empty? %s\n", list_empty(&fox_list) ? "empty" : "not empty");
    printf("list_is_singular? %s\n", list_is_singular(&fox_list) ? "singular" : "not singular");
}