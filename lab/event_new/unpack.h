#ifndef EV_UNPACK_H_
#define EV_UNPACK_H_

#include "sev.h"

int eio_unpack(evio_t* io, void* buf, int readbytes);
int eio_unpack_by_fixed_length(evio_t* io, void* buf, int readbytes);
int eio_unpack_by_delimiter(evio_t* io, void* buf, int readbytes);
int eio_unpack_by_length_field(evio_t* io, void* buf, int readbytes);

#endif // EV_UNPACK_H_
