#include "hash.h"
#include "test.h"

struct my_struct {
    int i1;
    int i2;
};

uint32_t my_struct_key(const void* arg) {
    const struct my_struct* ms = arg;

    return ms->i1;
}

bool my_struct_hash_equal(const void* arg1, const void* arg2) {
    const struct my_struct* ms1 = arg1;
    const struct my_struct* ms2 = arg2;

    if (ms1->i1 != ms2->i1)
        return false;

    return true;
}

void test_hash() {

    struct hash* h;
    h = hash_create(my_struct_key, my_struct_hash_equal, NULL);
    struct my_struct s1 = {1, 1};
    struct my_struct s2 = {2, 2};
    
    hash_get(h, &s1, hash_alloc_intern);
    hash_get(h, &s2, hash_alloc_intern);

    struct my_struct *res = NULL;
    res = (struct my_struct *)hash_lookup(h, &(struct my_struct){ .i1=1 });

    s1.i1 = 8;

    printf("%p\n", res);
    printf("%d, %d\n", res->i1, res->i2);
}
