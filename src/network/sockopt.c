#include "sockopt.h"
#include "socket.h"
#include <net/if.h>
#include <sys/ioctl.h>

int setsockopt_tos(int sockfd, int tos) {
    if (setsockopt(sockfd, IPPROTO_IP, IP_TOS, &tos, sizeof(tos)) < 0) {
        printe("can't setsockopt IP_TOS on fd %d for tos 0x%.2x\n", sockfd, tos);
        return -1;
    }
    return 0;
}

int getsockopt_tos(int sockfd) {
    int tos;
    socklen_t len;
    if (getsockopt(sockfd, IPPROTO_IP, IP_TOS, &tos, &len) < 0) {
        printe("can't getsockopt IP_TOS on fd %d\n", sockfd);
        return -1;
    }
    return tos;
}

int setsockopt_ttl(int sockfd, int ttl) {
    if (setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) < 0) {
        printe("can't setsockopt IP_TOS on fd %d for ttl %d\n", sockfd, ttl);
        return -1;
    }
    return 0;
}

int getsockopt_ttl(int sockfd) {
    int ttl;
    socklen_t len;
    if (getsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl, &len) < 0) {
        printe("can't getsockopt IP_TOS on fd %d\n", sockfd);
        return -1;
    }
    return ttl;
}

int setsockopt_recvttl(int sockfd, int on) {
    if (setsockopt(sockfd, IPPROTO_IP, IP_RECVTTL, &on, sizeof(on)) < 0) {
        printe("can't setsockopt IP_RECVTTL on fd TTL reception%d\n", sockfd);
        return -1;
    }
    return 0;
}

int setsockopt_recvtos(int sockfd, int on) {
    if (setsockopt(sockfd, IPPROTO_IP, IP_RECVTOS, &on, sizeof(on)) < 0) {
        printe("can't setsockopt IP_RECVTOS on fd TOS reception%d\n", sockfd);
        return -1;
    }
    return 0;
}

/*
 * Process multicast socket options for IPv4 in an OS-dependent manner.
 * Supported options are IP_{ADD,DROP}_MEMBERSHIP.
 *
 * Many operating systems have a limit on the number of groups that
 * can be joined per socket (where each group and local address
 * counts).  This impacts OSPF, which joins groups on each interface
 * using a single socket.  The limit is typically 20, derived from the
 * original BSD multicast implementation.  Some systems have
 * mechanisms for increasing this limit.
 *
 * In many 4.4BSD-derived systems, multicast group operations are not
 * allowed on interfaces that are not UP.  Thus, a previous attempt to
 * leave the group may have failed, leaving it still joined, and we
 * drop/join quietly to recover.  This may not be necessary, but aims to
 * defend against unknown behavior in that we will still return an error
 * if the second join fails.  It is not clear how other systems
 * (e.g. Linux, Solaris) behave when leaving groups on down interfaces,
 * but this behavior should not be harmful if they behave the same way,
 * allow leaves, or implicitly leave all groups joined to down interfaces.
 */
int setsockopt_ipv4_multicast(int sock, int optname, struct in_addr if_addr,
                              unsigned int mcast_addr, ifindex_t ifindex) {
    // #ifdef HAVE_RFC3678
    struct group_req gr;
    struct sockaddr_in* si;
    int ret;
    memset(&gr, 0, sizeof(gr));
    si = (struct sockaddr_in*)&gr.gr_group;
    gr.gr_interface = ifindex;
    si->sin_family = AF_INET;
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
    si->sin_len = sizeof(struct sockaddr_in);
#endif /* HAVE_STRUCT_SOCKADDR_IN_SIN_LEN */
    si->sin_addr.s_addr = mcast_addr;
    ret = setsockopt(sock, IPPROTO_IP,
                     (optname == IP_ADD_MEMBERSHIP) ? MCAST_JOIN_GROUP : MCAST_LEAVE_GROUP,
                     (void*)&gr, sizeof(gr));
    if ((ret < 0) && (optname == IP_ADD_MEMBERSHIP) && (errno == EADDRINUSE)) {
        setsockopt(sock, IPPROTO_IP, MCAST_LEAVE_GROUP, (void*)&gr, sizeof(gr));
        ret = setsockopt(sock, IPPROTO_IP, MCAST_JOIN_GROUP, (void*)&gr, sizeof(gr));
    }

    if (ret < 0) {
        printe("can't setsockopt IP_ADD_MEMBERSHIP %s\n", strerror(errno));
    }

    return ret;

    // #elif defined(HAVE_STRUCT_IP_MREQN_IMR_IFINDEX) && !defined(__FreeBSD__)
    // struct ip_mreqn mreqn;
    // 	int ret;

    // 	assert(optname == IP_ADD_MEMBERSHIP || optname == IP_DROP_MEMBERSHIP);
    // 	memset(&mreqn, 0, sizeof(mreqn));

    // 	mreqn.imr_multiaddr.s_addr = mcast_addr;
    // 	mreqn.imr_ifindex = ifindex;

    // 	ret = setsockopt(sock, IPPROTO_IP, optname, (void *)&mreqn,
    // 			 sizeof(mreqn));
    // 	if ((ret < 0) && (optname == IP_ADD_MEMBERSHIP)
    // 	    && (errno == EADDRINUSE)) {
    // 		/* see above: handle possible problem when interface comes back
    // 		 * up */
    // 		zlog_info(
    // 			"setsockopt_ipv4_multicast attempting to drop and re-add (fd %d, mcast %pI4, ifindex %u)",
    // 			sock, &mreqn.imr_multiaddr, ifindex);
    // 		setsockopt(sock, IPPROTO_IP, IP_DROP_MEMBERSHIP, (void *)&mreqn,
    // 			   sizeof(mreqn));
    // 		ret = setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
    // 				 (void *)&mreqn, sizeof(mreqn));
    // 	}
    // 	return ret;

    // /* Example defines for another OS, boilerplate off other code in this
    //    function, AND handle optname as per other sections for consistency !! */
    // /* #elif  defined(BOGON_NIX) && EXAMPLE_VERSION_CODE > -100000 */
    // /* Add your favourite OS here! */

    // #elif defined(HAVE_BSD_STRUCT_IP_MREQ_HACK) /* #if OS_TYPE */
    // 	/* standard BSD API */

    // struct ip_mreq mreq;
    // 	int ret;

    // 	assert(optname == IP_ADD_MEMBERSHIP || optname == IP_DROP_MEMBERSHIP);

    // 	memset(&mreq, 0, sizeof(mreq));
    // 	mreq.imr_multiaddr.s_addr = mcast_addr;
    // #if !defined __OpenBSD__
    // 	mreq.imr_interface.s_addr = htonl(ifindex);
    // #else
    // 	mreq.imr_interface.s_addr = if_addr.s_addr;
    // #endif

    // 	ret = setsockopt(sock, IPPROTO_IP, optname, (void *)&mreq,
    // 			 sizeof(mreq));
    // 	if ((ret < 0) && (optname == IP_ADD_MEMBERSHIP)
    // 	    && (errno == EADDRINUSE)) {
    // 		/* see above: handle possible problem when interface comes back
    // 		 * up */
    // 		zlog_info(
    // 			"setsockopt_ipv4_multicast attempting to drop and re-add (fd %d, mcast %pI4, ifindex %u)",
    // 			sock, &mreq.imr_multiaddr, ifindex);
    // 		setsockopt(sock, IPPROTO_IP, IP_DROP_MEMBERSHIP, (void *)&mreq,
    // 			   sizeof(mreq));
    // 		ret = setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
    // 				 (void *)&mreq, sizeof(mreq));
    // 	}
    // 	return ret;

    // #else
    // #error "Unsupported multicast API"
    // #endif /* #if OS_TYPE */
}

int so_bindtodev(int sock, char* if_name) {
    int ret;
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, if_name, IF_NAMESIZE);
    ret = setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, (char*)&ifr, sizeof(ifr));

    if (ret < 0) {
        printe("can't setsockopt SO_BINDTODEVICE on fd %d for interface %s\n", sock, if_name);
    }
    return 0;
}

#define TODO_BELOW
int mcast_get_if(int sockfd) {
    switch (sockfd_to_family(sockfd)) {
    case AF_INET: {
        /* TODO: similar to mcast_set_if() */
        return (-1);
    }

#ifdef IPV6
    case AF_INET6: {
        u_int idx;
        socklen_t len;

        len = sizeof(idx);
        if (getsockopt(sockfd, IPPROTO_IPV6, IPV6_MULTICAST_IF,
                       &idx, &len) < 0)
            return (-1);
        return (idx);
    }
#endif

    default:
        errno = EAFNOSUPPORT;
        return (-1);
    }
}

int Mcast_get_if(int sockfd) {
    int rc;

    if ((rc = mcast_get_if(sockfd)) < 0)
        err_sys("mcast_get_if error");
    return (rc);
}

int mcast_get_loop(int sockfd) {
    switch (sockfd_to_family(sockfd)) {
    case AF_INET: {
        u_char flag;
        socklen_t len;

        len = sizeof(flag);
        if (getsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_LOOP,
                       &flag, &len) < 0)
            return (-1);
        return (flag);
    }

#ifdef IPV6
    case AF_INET6: {
        u_int flag;
        socklen_t len;

        len = sizeof(flag);
        if (getsockopt(sockfd, IPPROTO_IPV6, IPV6_MULTICAST_LOOP,
                       &flag, &len) < 0)
            return (-1);
        return (flag);
    }
#endif

    default:
        errno = EAFNOSUPPORT;
        return (-1);
    }
}

int Mcast_get_loop(int sockfd) {
    int rc;

    if ((rc = mcast_get_loop(sockfd)) < 0)
        err_sys("mcast_get_loop error");
    return (rc);
}

int mcast_get_ttl(int sockfd) {
    switch (sockfd_to_family(sockfd)) {
    case AF_INET: {
        u_char ttl;
        socklen_t len;

        len = sizeof(ttl);
        if (getsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_TTL,
                       &ttl, &len) < 0)
            return (-1);
        return (ttl);
    }

#ifdef IPV6
    case AF_INET6: {
        int hop;
        socklen_t len;

        len = sizeof(hop);
        if (getsockopt(sockfd, IPPROTO_IPV6, IPV6_MULTICAST_HOPS,
                       &hop, &len) < 0)
            return (-1);
        return (hop);
    }
#endif

    default:
        errno = EAFNOSUPPORT;
        return (-1);
    }
}

int Mcast_get_ttl(int sockfd) {
    int rc;

    if ((rc = mcast_get_ttl(sockfd)) < 0)
        err_sys("mcast_get_ttl error");
    return (rc);
}

int mcast_set_if(int sockfd, const char* ifname, u_int ifindex) {
    switch (sockfd_to_family(sockfd)) {
    case AF_INET: {
        struct in_addr inaddr;
        struct ifreq ifreq;

        if (ifindex > 0) {
            if (if_indextoname(ifindex, ifreq.ifr_name) == NULL) {
                errno = ENXIO; /* i/f index not found */
                return (-1);
            }
            goto doioctl;
        } else if (ifname != NULL) {
            strncpy(ifreq.ifr_name, ifname, IFNAMSIZ);
        doioctl:
            if (ioctl(sockfd, SIOCGIFADDR, &ifreq) < 0)
                return (-1);
            memcpy(&inaddr,
                   &((struct sockaddr_in*)&ifreq.ifr_addr)->sin_addr,
                   sizeof(struct in_addr));
        } else
            inaddr.s_addr = htonl(INADDR_ANY); /* remove prev. set default */

        return (setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_IF,
                           &inaddr, sizeof(struct in_addr)));
    }

#ifdef IPV6
    case AF_INET6: {
        u_int idx;

        if ((idx = ifindex) == 0) {
            if (ifname == NULL) {
                errno = EINVAL; /* must supply either index or name */
                return (-1);
            }
            if ((idx = if_nametoindex(ifname)) == 0) {
                errno = ENXIO; /* i/f name not found */
                return (-1);
            }
        }
        return (setsockopt(sockfd, IPPROTO_IPV6, IPV6_MULTICAST_IF,
                           &idx, sizeof(idx)));
    }
#endif

    default:
        errno = EAFNOSUPPORT;
        return (-1);
    }
}

void Mcast_set_if(int sockfd, const char* ifname, u_int ifindex) {
    if (mcast_set_if(sockfd, ifname, ifindex) < 0)
        err_sys("mcast_set_if error");
    // printe("Can't setsockopt IP_MULTICAST_IF on fd %d to ifindex %d for interface %s\n",
    //        sock, ifindex, "TODO:indexname");
}

int mcast_set_loop(int sockfd, int onoff) {
    switch (sockfd_to_family(sockfd)) {
    case AF_INET: {
        u_char flag;

        flag = onoff;
        return (setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_LOOP,
                           &flag, sizeof(flag)));
    }

#ifdef IPV6
    case AF_INET6: {
        u_int flag;

        flag = onoff;
        return (setsockopt(sockfd, IPPROTO_IPV6, IPV6_MULTICAST_LOOP,
                           &flag, sizeof(flag)));
    }
#endif

    default:
        errno = EAFNOSUPPORT;
        return (-1);
    }
}
/* end mcast_set_loop */

void Mcast_set_loop(int sockfd, int onoff) {
    if (mcast_set_loop(sockfd, onoff) < 0)
        err_sys("mcast_set_loop error");
}

int mcast_set_ttl(int sockfd, int val) {
    switch (sockfd_to_family(sockfd)) {
    case AF_INET: {
        u_char ttl;

        ttl = val;
        return (setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_TTL,
                           &ttl, sizeof(ttl)));
    }

#ifdef IPV6
    case AF_INET6: {
        int hop;

        hop = val;
        return (setsockopt(sockfd, IPPROTO_IPV6, IPV6_MULTICAST_HOPS,
                           &hop, sizeof(hop)));
    }
#endif

    default:
        errno = EAFNOSUPPORT;
        return (-1);
    }
}

void Mcast_set_ttl(int sockfd, int val) {
    if (mcast_set_ttl(sockfd, val) < 0)
        err_sys("mcast_set_ttl error");
}

int mcast_join(int sockfd, const SA* grp, socklen_t grplen,
               const char* ifname, u_int ifindex) {
#ifdef MCAST_JOIN_GROUP
    struct group_req req;
    if (ifindex > 0) {
        req.gr_interface = ifindex;
    } else if (ifname != NULL) {
        if ((req.gr_interface = if_nametoindex(ifname)) == 0) {
            errno = ENXIO; /* i/f name not found */
            return (-1);
        }
    } else
        req.gr_interface = 0;
    if (grplen > sizeof(req.gr_group)) {
        errno = EINVAL;
        return -1;
    }
    memcpy(&req.gr_group, grp, grplen);
    return (setsockopt(sockfd, family_to_level(grp->sa_family),
                       MCAST_JOIN_GROUP, &req, sizeof(req)));
#else
    /* end mcast_join1 */

    /* include mcast_join2 */
    switch (grp->sa_family) {
    case AF_INET: {
        struct ip_mreq mreq;
        struct ifreq ifreq;

        memcpy(&mreq.imr_multiaddr,
               &((const struct sockaddr_in*)grp)->sin_addr,
               sizeof(struct in_addr));

        if (ifindex > 0) {
            if (if_indextoname(ifindex, ifreq.ifr_name) == NULL) {
                errno = ENXIO; /* i/f index not found */
                return (-1);
            }
            goto doioctl;
        } else if (ifname != NULL) {
            strncpy(ifreq.ifr_name, ifname, IFNAMSIZ);
        doioctl:
            if (ioctl(sockfd, SIOCGIFADDR, &ifreq) < 0)
                return (-1);
            memcpy(&mreq.imr_interface,
                   &((struct sockaddr_in*)&ifreq.ifr_addr)->sin_addr,
                   sizeof(struct in_addr));
        } else
            mreq.imr_interface.s_addr = htonl(INADDR_ANY);

        return (setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                           &mreq, sizeof(mreq)));
    }
/* end mcast_join2 */

/* include mcast_join3 */
#ifdef IPV6
    case AF_INET6: {
        struct ipv6_mreq mreq6;

        memcpy(&mreq6.ipv6mr_multiaddr,
               &((const struct sockaddr_in6*)grp)->sin6_addr,
               sizeof(struct in6_addr));

        if (ifindex > 0) {
            mreq6.ipv6mr_interface = ifindex;
        } else if (ifname != NULL) {
            if ((mreq6.ipv6mr_interface = if_nametoindex(ifname)) == 0) {
                errno = ENXIO; /* i/f name not found */
                return (-1);
            }
        } else
            mreq6.ipv6mr_interface = 0;

        return (setsockopt(sockfd, IPPROTO_IPV6, IPV6_JOIN_GROUP,
                           &mreq6, sizeof(mreq6)));
    }
#endif

    default:
        errno = EAFNOSUPPORT;
        return (-1);
    }
#endif
}
/* end mcast_join3 */

void Mcast_join(int sockfd, const SA* grp, socklen_t grplen,
                const char* ifname, u_int ifindex) {
    if (mcast_join(sockfd, grp, grplen, ifname, ifindex) < 0)
        err_sys("mcast_join error");
}

int mcast_join_source_group(int sockfd, const SA* src, socklen_t srclen,
                            const SA* grp, socklen_t grplen,
                            const char* ifname, u_int ifindex) {
#ifdef MCAST_JOIN_SOURCE_GROUP
    struct group_source_req req;
    if (ifindex > 0) {
        req.gsr_interface = ifindex;
    } else if (ifname != NULL) {
        if ((req.gsr_interface = if_nametoindex(ifname)) == 0) {
            errno = ENXIO; /* i/f name not found */
            return (-1);
        }
    } else
        req.gsr_interface = 0;
    if (grplen > sizeof(req.gsr_group) || srclen > sizeof(req.gsr_source)) {
        errno = EINVAL;
        return -1;
    }
    memcpy(&req.gsr_group, grp, grplen);
    memcpy(&req.gsr_source, src, srclen);
    return (setsockopt(sockfd, family_to_level(grp->sa_family),
                       MCAST_JOIN_SOURCE_GROUP, &req, sizeof(req)));
#else
    switch (grp->sa_family) {
#ifdef IP_ADD_SOURCE_MEMBERSHIP
    case AF_INET: {
        struct ip_mreq_source mreq;
        struct ifreq ifreq;

        memcpy(&mreq.imr_multiaddr,
               &((struct sockaddr_in*)grp)->sin_addr,
               sizeof(struct in_addr));
        memcpy(&mreq.imr_sourceaddr,
               &((struct sockaddr_in*)src)->sin_addr,
               sizeof(struct in_addr));

        if (ifindex > 0) {
            if (if_indextoname(ifindex, ifreq.ifr_name) == NULL) {
                errno = ENXIO; /* i/f index not found */
                return (-1);
            }
            goto doioctl;
        } else if (ifname != NULL) {
            strncpy(ifreq.ifr_name, ifname, IFNAMSIZ);
        doioctl:
            if (ioctl(sockfd, SIOCGIFADDR, &ifreq) < 0)
                return (-1);
            memcpy(&mreq.imr_interface,
                   &((struct sockaddr_in*)&ifreq.ifr_addr)->sin_addr,
                   sizeof(struct in_addr));
        } else
            mreq.imr_interface.s_addr = htonl(INADDR_ANY);

        return (setsockopt(sockfd, IPPROTO_IP, IP_ADD_SOURCE_MEMBERSHIP,
                           &mreq, sizeof(mreq)));
    }
#endif

#ifdef IPV6
    case AF_INET6: /* IPv6 source-specific API is MCAST_JOIN_SOURCE_GROUP */
#endif
    default:
        errno = EAFNOSUPPORT;
        return (-1);
    }
#endif
}

void Mcast_join_source_group(int sockfd, const SA* src, socklen_t srclen,
                             const SA* grp, socklen_t grplen,
                             const char* ifname, u_int ifindex) {
    if (mcast_join_source_group(sockfd, src, srclen, grp, grplen,
                                ifname, ifindex) < 0)
        err_sys("mcast_join_source_group error");
}

int mcast_block_source(int sockfd, const SA* src, socklen_t srclen,
                       const SA* grp, socklen_t grplen) {
#ifdef MCAST_BLOCK_SOURCE
    struct group_source_req req;
    req.gsr_interface = 0;
    if (grplen > sizeof(req.gsr_group) || srclen > sizeof(req.gsr_source)) {
        errno = EINVAL;
        return -1;
    }
    memcpy(&req.gsr_group, grp, grplen);
    memcpy(&req.gsr_source, src, srclen);
    return (setsockopt(sockfd, family_to_level(grp->sa_family),
                       MCAST_BLOCK_SOURCE, &req, sizeof(req)));
#else
    switch (grp->sa_family) {
#ifdef IP_BLOCK_SOURCE
    case AF_INET: {
        struct ip_mreq_source mreq;

        memcpy(&mreq.imr_multiaddr,
               &((struct sockaddr_in*)grp)->sin_addr,
               sizeof(struct in_addr));
        memcpy(&mreq.imr_sourceaddr,
               &((struct sockaddr_in*)src)->sin_addr,
               sizeof(struct in_addr));
        mreq.imr_interface.s_addr = htonl(INADDR_ANY);

        return (setsockopt(sockfd, IPPROTO_IP, IP_BLOCK_SOURCE,
                           &mreq, sizeof(mreq)));
    }
#endif

#ifdef IPV6
    case AF_INET6: /* IPv6 source-specific API is MCAST_BLOCK_SOURCE */
#endif
    default:
        errno = EAFNOSUPPORT;
        return (-1);
    }
#endif
}

void Mcast_block_source(int sockfd, const SA* src, socklen_t srclen,
                        const SA* grp, socklen_t grplen) {
    if (mcast_block_source(sockfd, src, srclen, grp, grplen) < 0)
        err_sys("mcast_block_source error");
}

int mcast_unblock_source(int sockfd, const SA* src, socklen_t srclen,
                         const SA* grp, socklen_t grplen) {
#ifdef MCAST_UNBLOCK_SOURCE
    struct group_source_req req;
    req.gsr_interface = 0;
    if (grplen > sizeof(req.gsr_group) || srclen > sizeof(req.gsr_source)) {
        errno = EINVAL;
        return -1;
    }
    memcpy(&req.gsr_group, grp, grplen);
    memcpy(&req.gsr_source, src, srclen);
    return (setsockopt(sockfd, family_to_level(grp->sa_family),
                       MCAST_UNBLOCK_SOURCE, &req, sizeof(req)));
#else
    switch (grp->sa_family) {
#ifdef IP_UNBLOCK_SOURCE
    case AF_INET: {
        struct ip_mreq_source mreq;

        memcpy(&mreq.imr_multiaddr,
               &((struct sockaddr_in*)grp)->sin_addr,
               sizeof(struct in_addr));
        memcpy(&mreq.imr_sourceaddr,
               &((struct sockaddr_in*)src)->sin_addr,
               sizeof(struct in_addr));
        mreq.imr_interface.s_addr = htonl(INADDR_ANY);

        return (setsockopt(sockfd, IPPROTO_IP, IP_UNBLOCK_SOURCE,
                           &mreq, sizeof(mreq)));
    }
#endif

#ifdef IPV6
    case AF_INET6: /* IPv6 source-specific API is MCAST_UNBLOCK_SOURCE */
#endif
    default:
        errno = EAFNOSUPPORT;
        return (-1);
    }
#endif
}

void Mcast_unblock_source(int sockfd, const SA* src, socklen_t srclen,
                          const SA* grp, socklen_t grplen) {
    if (mcast_unblock_source(sockfd, src, srclen, grp, grplen) < 0)
        err_sys("mcast_unblock_source error");
}

int mcast_leave(int sockfd, const SA* grp, socklen_t grplen) {
#ifdef MCAST_JOIN_GROUP
    struct group_req req;
    req.gr_interface = 0;
    if (grplen > sizeof(req.gr_group)) {
        errno = EINVAL;
        return -1;
    }
    memcpy(&req.gr_group, grp, grplen);
    return (setsockopt(sockfd, family_to_level(grp->sa_family),
                       MCAST_LEAVE_GROUP, &req, sizeof(req)));
#else
    switch (grp->sa_family) {
    case AF_INET: {
        struct ip_mreq mreq;

        memcpy(&mreq.imr_multiaddr,
               &((const struct sockaddr_in*)grp)->sin_addr,
               sizeof(struct in_addr));
        mreq.imr_interface.s_addr = htonl(INADDR_ANY);
        return (setsockopt(sockfd, IPPROTO_IP, IP_DROP_MEMBERSHIP,
                           &mreq, sizeof(mreq)));
    }

#ifdef IPV6
    case AF_INET6: {
        struct ipv6_mreq mreq6;

        memcpy(&mreq6.ipv6mr_multiaddr,
               &((const struct sockaddr_in6*)grp)->sin6_addr,
               sizeof(struct in6_addr));
        mreq6.ipv6mr_interface = 0;
        return (setsockopt(sockfd, IPPROTO_IPV6, IPV6_LEAVE_GROUP,
                           &mreq6, sizeof(mreq6)));
    }
#endif

    default:
        errno = EAFNOSUPPORT;
        return (-1);
    }
#endif
}

void Mcast_leave(int sockfd, const SA* grp, socklen_t grplen) {
    if (mcast_leave(sockfd, grp, grplen) < 0)
        err_sys("mcast_leave error");
}

int mcast_leave_source_group(int sockfd, const SA* src, socklen_t srclen,
                             const SA* grp, socklen_t grplen) {
#ifdef MCAST_LEAVE_SOURCE_GROUP
    struct group_source_req req;
    req.gsr_interface = 0;
    if (grplen > sizeof(req.gsr_group) || srclen > sizeof(req.gsr_source)) {
        errno = EINVAL;
        return -1;
    }
    memcpy(&req.gsr_group, grp, grplen);
    memcpy(&req.gsr_source, src, srclen);
    return (setsockopt(sockfd, family_to_level(grp->sa_family),
                       MCAST_LEAVE_SOURCE_GROUP, &req, sizeof(req)));
#else
    switch (grp->sa_family) {
#ifdef IP_DROP_SOURCE_MEMBERSHIP
    case AF_INET: {
        struct ip_mreq_source mreq;

        memcpy(&mreq.imr_multiaddr,
               &((struct sockaddr_in*)grp)->sin_addr,
               sizeof(struct in_addr));
        memcpy(&mreq.imr_sourceaddr,
               &((struct sockaddr_in*)src)->sin_addr,
               sizeof(struct in_addr));
        mreq.imr_interface.s_addr = htonl(INADDR_ANY);

        return (setsockopt(sockfd, IPPROTO_IP, IP_DROP_SOURCE_MEMBERSHIP,
                           &mreq, sizeof(mreq)));
    }
#endif

#ifdef IPV6
    case AF_INET6: /* IPv6 source-specific API is MCAST_LEAVE_SOURCE_GROUP */
#endif
    default:
        errno = EAFNOSUPPORT;
        return (-1);
    }
#endif
}

void Mcast_leave_source_group(int sockfd, const SA* src, socklen_t srclen,
                              const SA* grp, socklen_t grplen) {
    if (mcast_leave_source_group(sockfd, src, srclen, grp, grplen) < 0)
        err_sys("mcast_leave_source_group error");
}