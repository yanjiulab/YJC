#include "inet.h"
#include "log.h"
#include "test.h"
#include "vector.h"
void test_vector() {
    void *val;
    int i;
    // create vec
    // vector(int) *i_vec_p = vec_new(int);
    // printf("new i_vec_p: %p\n", i_vec_p);
    // vector(char *) str_vec;
    // vec_init(&str_vec);
    // printf("new str_vec: %p\n", &str_vec);

    // // add
    // vec_push(i_vec_p, 1);
    // vec_push(i_vec_p, 1);
    // vec_push(i_vec_p, 2);
    // vec_push(i_vec_p, 3);

    // vec_insert(&str_vec, 0, "a");
    // vec_insert(&str_vec, 1, "bc");
    // vec_insert(&str_vec, 2, "def");
    // vec_insert(&str_vec, 3, "1a3b");
    // vec_insert(&str_vec, 4, "bc");

    // // foreach
    // int var, iter;
    // vec_foreach(i_vec_p, var, iter) { printf("%d ", var); }
    // printf("\n");

    // char *str;
    // vec_foreach(&str_vec, str, iter) { printf("%s ", str); }
    // printf("\n");

    // // del
    // vec_pop(i_vec_p);
    // vec_dump(i_vec_p, "%d");

    // vec_remove(&str_vec, "bc");
    // vec_dump(&str_vec, "%s");

    // // destroy vec
    // vec_deinit(&str_vec);
    // vec_free(i_vec_p);

    vector v = vector_init(10);
    int a = 1, b = 2;

    vector_set(v, "a11");
    vector_set(v, "b21");
    vector_set(v, "e31");
    vector_set(v, "f41");
    // vector_unset(v, 1);
    // vector_unset(v, 2);
    vector_remove(v, 1);
    vector_remove(v, 2);
    vector_push(v, "a51");
    vector_push(v, "c61");
    // vector_insert(v, 7, 7);
    vector_push(v, "b81");
    vector_set(v, "211");
    // vector_set(v, 7);
    // vector_set_index(v, 1, &b);
    // int *l = vector_lookup(v, 0);

    // log_info("%d", *l);
    log_info("next empty idx:%d", vector_empty_slot(v));
    log_info("active:%d", vector_active(v));
    log_info("count:%d", vector_count(v));
    log_info("alloced:%d", v->alloced);
    print_data(v->index, v->active * sizeof(void *));
    // vector_reverse(v);
    vector_foreach(v, val, i) { printf("%s ", val); }
    printf("\n");

    vector_sort(v, vector_str_cmp);

    print_data(v->index, v->active * sizeof(void *));
    vector_foreach(v, val, i) { printf("%s ", val); }
    printf("\n");
    // vector_foreach_rev(v, val, i) { printf("%s ", val); }
    // printf("\n");
    printf("%s\n", vector_slot(v, 0));

}