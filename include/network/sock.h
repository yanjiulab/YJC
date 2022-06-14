#ifndef SOCK_H
#define SOCK_H

#include <sys/socket.h>

#define LISTENQ 1024

/* socket creation */
int sock_packet();  // Ethernet frame (including ethernet header)
int sock_raw();     // IP datagram (including IP header)
int sock_udp();     // UDP datagram ()
int sock_tcp();     //

/*  */


/* util function */
const char *str_fam(int family);
const char *str_sock(int socktype);

#endif  // !SOCK_H
