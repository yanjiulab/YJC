#include "ipa.h"

#include <netdb.h>
#include <stdbool.h>

#include "debug.h"

// struct sockaddr -> char * (ip:port)
char *sock_ntop_host(const struct sockaddr *sa, socklen_t salen) {
    char portstr[8];
    static char str[128]; /* Unix domain is largest */

    switch (sa->sa_family) {
        case AF_INET: {
            struct sockaddr_in *sin = (struct sockaddr_in *)sa;

            if (inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str)) == NULL) return (NULL);
            if (ntohs(sin->sin_port) != 0) {
                snprintf(portstr, sizeof(portstr), ":%d", ntohs(sin->sin_port));
                strcat(str, portstr);
            }
            return (str);
        }

        case AF_INET6: {
            struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)sa;

            str[0] = '[';
            if (inet_ntop(AF_INET6, &sin6->sin6_addr, str + 1, sizeof(str) - 1) == NULL) return (NULL);
            if (ntohs(sin6->sin6_port) != 0) {
                snprintf(portstr, sizeof(portstr), "]:%d", ntohs(sin6->sin6_port));
                strcat(str, portstr);
                return (str);
            }
            return (str + 1);
        }

        case AF_UNIX: {
            struct sockaddr_un *unp = (struct sockaddr_un *)sa;

            /* OK to have no pathname bound to the socket: happens on
                       every connect() unless client calls bind() first. */
            if (unp->sun_path[0] == 0)
                strcpy(str, "(no pathname bound)");
            else
                snprintf(str, sizeof(str), "%s", unp->sun_path);
            return (str);
        }

#ifdef HAVE_SOCKADDR_DL_STRUCT
        case AF_LINK: {
            struct sockaddr_dl *sdl = (struct sockaddr_dl *)sa;

            if (sdl->sdl_nlen > 0)
                snprintf(str, sizeof(str), "%*s (index %d)", sdl->sdl_nlen, &sdl->sdl_data[0], sdl->sdl_index);
            else
                snprintf(str, sizeof(str), "AF_LINK, index=%d", sdl->sdl_index);
            return (str);
        }
#endif
        default:
            snprintf(str, sizeof(str), "sock_ntop: unknown AF_xxx: %d, len %d", sa->sa_family, salen);
            return (str);
    }
    return (NULL);
}

// struct sockaddr -> string
char *sock_ntop(const struct sockaddr *sa, socklen_t salen) {
    static char str[128]; /* Unix domain is largest */

    switch (sa->sa_family) {
        case AF_INET: {
            struct sockaddr_in *sin = (struct sockaddr_in *)sa;

            if (inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str)) == 0) return (NULL);
            return (str);
        }

#ifdef IPV6
        case AF_INET6: {
            struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)sa;

            if (inet_ntop(AF_INET6, &sin6->sin6_addr, str, sizeof(str)) == NULL) return (NULL);
            return (str);
        }
#endif

#ifdef AF_UNIX
        case AF_UNIX: {
            struct sockaddr_un *unp = (struct sockaddr_un *)sa;

            /* OK to have no pathname bound to the socket: happens on
                       every connect() unless client calls bind() first. */
            if (unp->sun_path[0] == 0)
                strcpy(str, "(no pathname bound)");
            else
                snprintf(str, sizeof(str), "%s", unp->sun_path);
            return (str);
        }
#endif

#ifdef HAVE_SOCKADDR_DL_STRUCT
        case AF_LINK: {
            struct sockaddr_dl *sdl = (struct sockaddr_dl *)sa;

            if (sdl->sdl_nlen > 0)
                snprintf(str, sizeof(str), "%*s", sdl->sdl_nlen, &sdl->sdl_data[0]);
            else
                snprintf(str, sizeof(str), "AF_LINK, index=%d", sdl->sdl_index);
            return (str);
        }
#endif
        default:
            snprintf(str, sizeof(str), "sock_ntop_host: unknown AF_xxx: %d, len %d", sa->sa_family, salen);
            return (str);
    }
    return (NULL);
}

char *Sock_ntop(const struct sockaddr *sa, socklen_t salen) {
    char *ptr;

    if ((ptr = sock_ntop(sa, salen)) == NULL) err_sys("sock_ntop error"); /* inet_ntop() sets errno */
    return (ptr);
}

// uint32_t <-> string
char *sock_itop(in_addr_t ip_int) {
    static char str[128];
    u_char *a = (u_char *)&ip_int;
    snprintf(str, INET_ADDRSTRLEN, "%u.%u.%u.%u", a[0], a[1], a[2], a[3]);
    return str;
}

in_addr_t sock_ptoi(const char *ip_str) { return inet_addr(ip_str); }

//

/*
 * Verify that a given netmask is plausible;
 * make sure that it is a series of 1's followed by
 * a series of 0's with no discontiguous 1's.
 */
int inet_valid_mask(uint32_t mask) {
    if (~(((mask & -mask) - 1) | mask) != 0) {
        return false;
    }

    return true;
}
