#include "test.h"
#include "vector.h"

void test_vector() {
    // create vec
    vector(int) *i_vec_p = vec_new(int);
    printf("new i_vec_p: %p\n", i_vec_p);
    vector(char *) str_vec;
    vec_init(&str_vec);
    printf("new str_vec: %p\n", &str_vec);

    // add
    vec_push(i_vec_p, 1);
    vec_push(i_vec_p, 1);
    vec_push(i_vec_p, 2);
    vec_push(i_vec_p, 3);

    vec_insert(&str_vec, 0, "a");
    vec_insert(&str_vec, 1, "bc");
    vec_insert(&str_vec, 2, "def");
    vec_insert(&str_vec, 3, "1a3b");
    vec_insert(&str_vec, 4, "bc");

    // foreach
    int var, iter;
    vec_foreach(i_vec_p, var, iter) { printf("%d ", var); }
    printf("\n");

    char *str;
    vec_foreach(&str_vec, str, iter) { printf("%s ", str); }
    printf("\n");

    // del
    vec_pop(i_vec_p);
    vec_dump(i_vec_p, "%d");

    vec_remove(&str_vec, "bc");
    vec_dump(&str_vec, "%s");

    // destroy vec
    vec_deinit(&str_vec);
    vec_free(i_vec_p);
}