#include "base.h"
#include "test.h"

void test_base() {
    char *p;
    MEMCHECK;
    EV_ALLOC(p, 100);
    strcpy(p, "hello world");
    EV_FREE(p);

    int i = 0;
    char *ptr[10] = {0};
    int size = 16, nsize = 32, osize = 16;
    ptr[i++] = ALLOC(size);
    ptr[i++] = ALLOC_SIZEOF(*ptr);
    ptr[i++] = MALLOC(size);
    ptr[i++] = CALLOC(2, size);
    ptr[i++] = REALLOC(ptr[i], nsize);          // printf("ptr:%p\n", ptr[i]);
    ptr[i++] = ZREALLOC(ptr[i], nsize, osize);  // printf("ptr:%p\n", ptr[i]);
    
    i = 0;
    for (i = 0; i < 10; i++) {
        printf("ptr:%p\n", ptr[i]);
        FREE(ptr[i]);
    }
}