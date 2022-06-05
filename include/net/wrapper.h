#ifndef WRAPPER_H
#define WRAPPER_H

#include <memory.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

void *Malloc(size_t size);
int Ioctl(int fd, int request, void *arg);
void *Calloc(size_t n, size_t size);

#endif