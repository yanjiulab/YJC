#ifndef __NL_DEBUG_H__
#define __NL_DEBUG_H__

/* Netlink debug */

#define NIPQUAD(addr) ((unsigned char*)&addr)[0], \
                      ((unsigned char*)&addr)[1], \
                      ((unsigned char*)&addr)[2], \
                      ((unsigned char*)&addr)[3]
#define NIPQUAD_FMT "%u.%u.%u.%u"

#ifndef s6_addr16
#define s6_addr16 __in6_u.__u6_addr16
#endif

#define NIP6(addr) ntohs((addr).s6_addr16[0]), \
                   ntohs((addr).s6_addr16[1]), \
                   ntohs((addr).s6_addr16[2]), \
                   ntohs((addr).s6_addr16[3]), \
                   ntohs((addr).s6_addr16[4]), \
                   ntohs((addr).s6_addr16[5]), \
                   ntohs((addr).s6_addr16[6]), \
                   ntohs((addr).s6_addr16[7])
#define NIP6_FMT "%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x"
#define NIP6_SEQFMT "%04x%04x%04x%04x%04x%04x%04x%04x"

const char* nlmsg_type2str(uint16_t type);
const char* af_type2str(int type);
const char* ifi_type2str(int type);
const char* rta_type2str(int type);
const char* rtm_type2str(int type);
const char* ifla_pdr_type2str(int type);
const char* ifla_info_type2str(int type);
const char* rtm_protocol2str(int type);
const char* rtm_scope2str(int type);
const char* rtm_rta2str(int type);
const char* neigh_rta2str(int type);
const char* ifa_rta2str(int type);
const char* nhm_rta2str(int type);
const char* frh_rta2str(int type);
const char* frh_action2str(uint8_t action);
const char* nlmsg_flags2str(uint16_t flags, char* buf, size_t buflen);
const char* if_flags2str(uint32_t flags, char* buf, size_t buflen);
const char* rtm_flags2str(uint32_t flags, char* buf, size_t buflen);
const char* neigh_state2str(uint32_t flags, char* buf, size_t buflen);
const char* neigh_flags2str(uint32_t flags, char* buf, size_t buflen);
const char* ifa_flags2str(uint32_t flags, char* buf, size_t buflen);
const char* nh_flags2str(uint32_t flags, char* buf, size_t buflen);

void nl_dump(void* msg, size_t msglen);

#endif