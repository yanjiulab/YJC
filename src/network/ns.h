// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * NS related header.
 * Copyright (C) 2014 6WIND S.A.
 */

#ifndef _ZEBRA_NS_H
#define _ZEBRA_NS_H

#include "linklist.h"
#include "openbsd_tree.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t ns_id_t;

/* the default NS ID */
#define NS_UNKNOWN UINT32_MAX

/* Default netns directory (Linux) */
#define NS_RUN_DIR "/var/run/netns"

#ifdef HAVE_NETNS
#define NS_DEFAULT_NAME "/proc/self/ns/net"
#else /* !HAVE_NETNS */
#define NS_DEFAULT_NAME "default-netns"
#endif /* HAVE_NETNS */

struct ns {
    RB_ENTRY(ns)
    entry;

    /* Identifier, same as the vector index */
    ns_id_t ns_id;

    /* Identifier, mapped on the NSID value */
    ns_id_t internal_ns_id;

    /* Identifier, value of NSID of default netns,
     * relative value in that local netns
     */
    ns_id_t relative_default_ns;

    /* Name */
    char* name;

    /* File descriptor */
    int fd;

    /* Master list of interfaces belonging to this NS */
    struct list* iflist;

    /* Back Pointer to VRF */
    void* vrf_ctxt;

    /* User data */
    void* info;
};
RB_HEAD(ns_head, ns);
RB_PROTOTYPE(ns_head, ns, entry, ns_compare)

/*
 * API for managing NETNS. eg from zebra daemon
 * one want to manage the list of NETNS, etc...
 */

/*
 * NS hooks
 */

#define NS_NEW_HOOK     0 /* a new netns is just created */
#define NS_DELETE_HOOK  1 /* a netns is to be deleted */
#define NS_ENABLE_HOOK  2 /* a netns is ready to use */
#define NS_DISABLE_HOOK 3 /* a netns is to be unusable */

/*
 * Add a specific hook ns module.
 * @param1: hook type
 * @param2: the callback function
 *          - param 1: the NS ID
 *          - param 2: the address of the user data pointer (the user data
 *                     can be stored in or freed from there)
 */
extern void ns_add_hook(int type, int (*)(struct ns*));

/*
 * NS initializer/destructor
 */

extern void ns_terminate(void);

/* API to initialize NETNS managerment
 * parameter is the default ns_id
 */
extern void ns_init_management(ns_id_t ns_id, ns_id_t internal_ns_idx);

/*
 * NS utilities
 */

/* Create a socket serving for the given NS
 */
int ns_socket(int domain, int type, int protocol, ns_id_t ns_id);

/* return the path of the NETNS */
extern char* ns_netns_pathname(const char* name);

/* Parse and execute a function on all the NETNS */
#define NS_WALK_CONTINUE 0
#define NS_WALK_STOP     1

extern void ns_walk_func(int (*func)(struct ns*,
                                     void*,
                                     void**),
                         void* param_in,
                         void** param_out);

/* API to get the NETNS name, from the ns pointer */
extern const char* ns_get_name(struct ns* ns);

/* only called from vrf ( when removing netns from vrf)
 * or at VRF termination
 */
extern void ns_delete(struct ns* ns);

/* return > 0 if netns is available
 * called by VRF to check netns backend is available for VRF
 */
extern int ns_have_netns(void);

/* API to get context information of a NS */
extern void* ns_info_lookup(ns_id_t ns_id);

/* API to map internal ns id value with
 * user friendly ns id external value
 */
extern ns_id_t ns_map_nsid_with_external(ns_id_t ns_id, bool map);

/*
 * NS init routine
 * should be called from backendx
 */
extern void ns_init(void);

#define NS_DEFAULT 0

/* API that can be used to change from NS */
extern int ns_switchback_to_initial(void);
extern int ns_switch_to_netns(const char* netns_name);

/*
 * NS handling routines.
 * called by modules that use NS backend
 */

/* API to search for already present NETNS */
extern struct ns* ns_lookup(ns_id_t ns_id);
extern struct ns* ns_lookup_name(const char* name);

/* API to handle NS : creation, enable, disable
 * for enable, a callback function is passed as parameter
 * the callback belongs to the module that uses NS as backend
 * upon enabling the NETNS, the upper layer is informed
 */
extern int ns_enable(struct ns* ns, void (*func)(ns_id_t, void*));
extern struct ns* ns_get_created(struct ns* ns, char* name, ns_id_t ns_id);
extern ns_id_t ns_id_get_absolute(ns_id_t ns_id_reference, ns_id_t link_nsid);
extern void ns_disable(struct ns* ns);
extern struct ns* ns_get_default(void);

#ifdef __cplusplus
}
#endif

#endif /*_ZEBRA_NS_H*/
