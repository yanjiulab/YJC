#ifndef EV_UNPACK_H_
#define EV_UNPACK_H_

#include "eventloop.h"

int eio_unpack(eio_t* io, void* buf, int readbytes);
int eio_unpack_by_fixed_length(eio_t* io, void* buf, int readbytes);
int eio_unpack_by_delimiter(eio_t* io, void* buf, int readbytes);
int eio_unpack_by_length_field(eio_t* io, void* buf, int readbytes);

#endif // EV_UNPACK_H_
