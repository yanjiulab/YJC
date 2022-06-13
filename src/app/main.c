#include <net/ethernet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

#include "ifi.h"
#include "ipa.h"
#include "str.h"
#include "timer.h"
// #include "ether.h"
// #include "arp.h"
// #include "ip.h"
// #include "icmp.h"
// #include "tcp.h"
// #include "udp.h"
// #include "pim.h"
// #include "rip.h"
#include "cliser.h"

int main(int argc, char *argv[]) {
    printf("Hello YJC\n");

    for (int i = 0; i < argc; i++) {
        printf("args(%d/%d): %s\n", i, argc, argv[i]);
    }

    return 0;
}