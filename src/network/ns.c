#include "ns.h"

#define _GNU_SOURCE

#include <sched.h>

/* Create a socket for the NS. */
// int ns_socket(int domain, int type, int protocol, ns_id_t ns_id) {
    // struct ns* ns = ns_lookup(ns_id);
    // int ret;

    // if (!ns || !ns_is_enabled(ns)) {
    //     errno = EINVAL;
    //     return -1;
    // }
    // if (have_netns()) {
    //     ret = (ns_id != NS_DEFAULT) ? setns(ns->fd, CLONE_NEWNET) : 0;
    //     if (ret >= 0) {
    //         ret = socket(domain, type, protocol);
    //         if (ns_id != NS_DEFAULT) {
    //             setns(ns_lookup(NS_DEFAULT)->fd, CLONE_NEWNET);
    //             ns_current_ns_fd = ns_id;
    //         }
    //     }
    // } else
    //     ret = socket(domain, type, protocol);

    // return ret;
// }
