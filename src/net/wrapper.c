#include "wrapper.h"

/* wrap sys/socket.h */


/* wrap stdlib.h */
void *Malloc(size_t size) {
    void *ptr;

    if ((ptr = malloc(size)) == NULL)
        err_sys("malloc error");
    return (ptr);
}

int Ioctl(int fd, int request, void *arg) {
    int n;

    if ((n = ioctl(fd, request, arg)) == -1)
        err_sys("ioctl error");
    return (n); /* streamio of I_LIST returns value */
}

void *Calloc(size_t n, size_t size) {
    void *ptr;

    if ((ptr = calloc(n, size)) == NULL)
        err_sys("calloc error");
    return (ptr);
}