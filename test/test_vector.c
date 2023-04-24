#include "inet.h"
#include "log.h"
#include "test.h"
#include "vector.h"
void test_vector() {
    vector v;
    void *val;
    int i;
    printf("-------------- int vector test --------------\n");
    v = vector_init(10);
    vector_push(v, 11);
    vector_push(v, 100);
    vector_push(v, 14);
    vector_push(v, 7);
    vector_push(v, 0);
    vector_push(v, -12);

    printf("vector: ");
    vector_foreach(v, val, i) { printf("%d ", val); }
    printf("\n");
    vector_reverse(v);
    printf("vector reverse: ");
    vector_foreach(v, val, i) { printf("%d ", val); }
    printf("\n");
    vector_insert(v, 4, 0);
    vector_insert(v, 6, 123);
    vector_insert(v, 1000, 10);
    vector_remove(v, 1);
    vector_unset_value(v, 100);
    printf("vector: ");
    vector_foreach(v, val, i) { printf("%d ", val); }
    printf("\n");
    vector_sort(v, vector_int_cmp);
    printf("vector sort: ");
    vector_foreach(v, val, i) { printf("%d ", val); }
    printf("\n");
    int a = 14;
    printf("vector find %d, result: %d\n", 47, vector_find(v, 47, vector_int_cmp));
    printf("vector find %d, result: %d\n", a, vector_find(v, a, vector_int_cmp));
    printf("vector find %d, result: %d\n", -12, vector_find(v, -12, vector_int_cmp));
    printf("vector find %d, result: %d\n", 86, vector_find(v, 86, vector_int_cmp));
    printf("vector next empty idx:%d\n", vector_empty_slot(v));
    printf("vector active:%d\n", vector_active(v));
    printf("vector count:%d\n", vector_count(v));
    printf("vector alloced:%d\n", v->alloced);
    printf("vector hex dump:\n");
    print_data((unsigned char *)v->index, v->active * sizeof(void *));

    // vector_free(v);

    printf("-------------- double vector test --------------\n");
    // v = vector_init(10);
    // vector_push(v, (void *)11.04);
    // vector_push(v, (void *)1.1);
    // vector_push(v, (void *)0.346);
    // vector_push(v, (void *)1e-5);
    // vector_push(v, (void *)1e-6);
    // vector_push(v, (void *)1e3);
    // printf("vector: ");
    // vector_foreach(v, val, i) { printf("%d ", val); }
    // printf("\n");
    // vector_reverse(v);
    // printf("vector reverse: ");
    // vector_foreach(v, val, i) { printf("%d ", val); }
    // printf("\n");
    // vector_sort(v, vector_int_cmp);
    // printf("vector sort: ");
    // vector_foreach(v, val, i) { printf("%d ", val); }
    // printf("\n");
    // int a = 14;
    // printf("vector find %d, result: %d\n", 47, vector_find(v, 47, vector_int_cmp));
    // printf("vector find %d, result: %d\n", a, vector_find(v, a, vector_int_cmp));
    // printf("vector find %d, result: %d\n", -12, vector_find(v, -12, vector_int_cmp));
    // printf("vector find %d, result: %d\n", 86, vector_find(v, 86, vector_int_cmp));

    // vector_free(v);
    printf("-------------- str vector test --------------\n");
    v = vector_init(10);
    vector_set(v, "a11");
    vector_set(v, "b21");
    vector_set(v, "e31");
    vector_set(v, "f41");
    vector_remove(v, 1);
    vector_remove(v, 2);
    vector_push(v, "a51");
    vector_push(v, "c61");
    vector_insert(v, 7, "8981");
    vector_push(v, "b81");
    vector_pop(v);
    vector_set(v, "211");
    vector_set_index(v, 1, "ac");
    printf("vector next empty idx:%d\n", vector_empty_slot(v));
    printf("vector active:%d\n", vector_active(v));
    printf("vector count:%d\n", vector_count(v));
    printf("vector alloced:%d\n", v->alloced);
    printf("vector hex dump:\n");
    print_data((unsigned char *)v->index, v->active * sizeof(void *));
    printf("vector: ");
    vector_foreach(v, val, i) { printf("%s ", val); }
    printf("\n");

    vector_reverse(v);
    printf("vector reverse: ");
    vector_foreach(v, val, i) { printf("%s ", val); }
    printf("\n");

    vector_sort(v, vector_str_cmp);
    printf("vector sort: ");
    vector_foreach(v, val, i) { printf("%s ", val); }
    printf("\n");

    char find[10] = "8981";
    printf("vector find %s, result: %d\n", "a11", vector_find(v, "a11", vector_str_cmp));
    printf("vector find %s, result: %d\n", find, vector_find(v, find, vector_str_cmp));
    printf("vector find %s, result: %d\n", "e31", vector_find(v, "e31", vector_str_cmp));
    printf("vector find %s, result: %d\n", "a51", vector_find(v, "a51", vector_str_cmp));
    printf("vector_get index 4: %s\n", vector_get(v, 4));
    vector_free(v);

    printf("-------------- custom vector test --------------\n");
}