/* tcp.h */

#ifndef __TCP_HEADERS_H__
#define __TCP_HEADERS_H__

#include <netinet/tcp.h>

#include "types.h"

/* TCP socket options used by Linux kernels under test but not in
 * standard Linux header files.
 */
#define SO_REUSEPORT             15

/* TCP socket options used by Linux kernels under test but not in
 * standard Linux header files.
 */
#define TCP_COOKIE_TRANSACTIONS  15 /* TCP Cookie Transactions */
#define TCP_THIN_LINEAR_TIMEOUTS 16 /* Use linear timeouts for thin streams */
#define TCP_THIN_DUPACK          17 /* Fast retrans. after 1 dupack */
#define TCP_USER_TIMEOUT         18 /* How long to retry losses */
#define TCP_FASTOPEN             23 /* TCP Fast Open: data in SYN */
#define TCP_TIMESTAMP            24
#define TCP_NOTSENT_LOWAT        25 /* limit unsent bytes in write queue */
#define TCP_CC_INFO              26 /* Get Congestion Control (optional) info */
#define TCP_SAVE_SYN             27 /* Record SYN headers for new connections */
#define TCP_SAVED_SYN            28 /* Get SYN headers recorded for connection */
#define TCP_REPAIR_WINDOW        29 /* Get/set window parameters */
#define TCP_FASTOPEN_CONNECT     30 /* Attempt FastOpen with connect */
#define TCP_FASTOPEN_KEY         33 /* Set the key for Fast Open (cookie) */
#define TCP_FASTOPEN_NO_COOKIE   34 /* Enable TFO without a TFO cookie */

#ifndef TCP_INQ
#define TCP_INQ    36
#define TCP_CM_INQ TCP_INQ
#endif

#define TCP_TX_DELAY 37

/* TODO: remove these when netinet/tcp.h has them */
#ifndef TCPI_OPT_ECN_SEEN
#define TCPI_OPT_ECN_SEEN 16 /* received at least one packet with ECT */
#endif
#ifndef TCPI_OPT_SYN_DATA
#define TCPI_OPT_SYN_DATA 32 /* SYN-ACK acked data in SYN sent or rcvd */
#endif

/* New TCP flags for sendto(2)/sendmsg(2). */
#ifndef MSG_FASTOPEN
#define MSG_FASTOPEN 0x20000000 /* TCP Fast Open: data in SYN */
#endif

#ifndef MSG_ZEROCOPY
#define MSG_ZEROCOPY 0x4000000
#endif

/* TCP option numbers and lengths. */
#define TCPOPT_EOL             0
#define TCPOPT_NOP             1
#define TCPOPT_MAXSEG          2
#define TCPOLEN_MAXSEG         4
#define TCPOPT_WINDOW          3
#define TCPOLEN_WINDOW         3
#define TCPOPT_SACK_PERMITTED  4
#define TCPOLEN_SACK_PERMITTED 2
#define TCPOPT_SACK            5
#define TCPOPT_TIMESTAMP       8
#define TCPOLEN_TIMESTAMP      10
#define TCPOPT_MD5SIG          19 /* MD5 Signature (RFC2385) */
#define TCPOLEN_MD5SIG         18
#define TCPOLEN_MD5_BASE       2
#define TCPOPT_FASTOPEN        34
#define TCPOPT_EXP             254 /* Experimental */

#define TCP_MD5_DIGEST_LEN     16 /* bytes in RFC2385 TCP MD5 digest */

/* A portable TCP header definition (Linux and *BSD use different names). */
struct tcp {
    uint16_t src_port;
    uint16_t dst_port;
    uint32_t seq;
    uint32_t ack_seq;
#if __BYTE_ORDER == __LITTLE_ENDIAN
    uint16_t ae : 1, res1 : 3, doff : 4, fin : 1, syn : 1, rst : 1, psh : 1,
        ack : 1, urg : 1, ece : 1, cwr : 1;
#elif __BYTE_ORDER == __BIG_ENDIAN
    uint16_t doff : 4, res1 : 3, ae : 1, cwr : 1, ece : 1, urg : 1, ack : 1,
        psh : 1, rst : 1, syn : 1, fin : 1;
#else
#error "Adjust your defines"
#endif
    uint16_t window;
    uint16_t check;
    uint16_t urg_ptr;
};

#ifdef linux

/* Data returned by the TCP_INFO socket option. */
struct _tcp_info {
    uint8_t tcpi_state;
    uint8_t tcpi_ca_state;
    uint8_t tcpi_retransmits;
    uint8_t tcpi_probes;
    uint8_t tcpi_backoff;
    uint8_t tcpi_options;
    uint8_t tcpi_snd_wscale : 4, tcpi_rcv_wscale : 4;
    uint8_t tcpi_delivery_rate_app_limited : 1;

    uint32_t tcpi_rto;
    uint32_t tcpi_ato;
    uint32_t tcpi_snd_mss;
    uint32_t tcpi_rcv_mss;

    uint32_t tcpi_unacked;
    uint32_t tcpi_sacked;
    uint32_t tcpi_lost;
    uint32_t tcpi_retrans;
    uint32_t tcpi_fackets;

    /* Times. */
    uint32_t tcpi_last_data_sent;
    uint32_t tcpi_last_ack_sent; /* Not remembered, sorry. */
    uint32_t tcpi_last_data_recv;
    uint32_t tcpi_last_ack_recv;

    /* Metrics. */
    uint32_t tcpi_pmtu;
    uint32_t tcpi_rcv_ssthresh;
    uint32_t tcpi_rtt;
    uint32_t tcpi_rttvar;
    uint32_t tcpi_snd_ssthresh;
    uint32_t tcpi_snd_cwnd;
    uint32_t tcpi_advmss;
    uint32_t tcpi_reordering;

    uint32_t tcpi_rcv_rtt;
    uint32_t tcpi_rcv_space;

    uint32_t tcpi_total_retrans;

    uint64_t tcpi_pacing_rate;
    uint64_t tcpi_max_pacing_rate;
    uint64_t tcpi_bytes_acked;    /* RFC4898 tcpEStatsAppHCThruOctetsAcked */
    uint64_t tcpi_bytes_received; /* RFC4898 tcpEStatsAppHCThruOctetsReceived */
    uint32_t tcpi_segs_out;       /* RFC4898 tcpEStatsPerfSegsOut */
    uint32_t tcpi_segs_in;        /* RFC4898 tcpEStatsPerfSegsIn */

    uint32_t tcpi_notsent_bytes;
    uint32_t tcpi_min_rtt;
    uint32_t tcpi_data_segs_in;  /* RFC4898 tcpEStatsDataSegsIn */
    uint32_t tcpi_data_segs_out; /* RFC4898 tcpEStatsDataSegsOut */
    uint64_t tcpi_delivery_rate;

    uint64_t tcpi_busy_time;      /* Time (usec) busy sending data */
    uint64_t tcpi_rwnd_limited;   /* Time (usec) limited by receive window */
    uint64_t tcpi_sndbuf_limited; /* Time (usec) limited by send buffer */

    uint32_t tcpi_delivered;
    uint32_t tcpi_delivered_ce;

    uint64_t tcpi_bytes_sent;    /* RFC4898 tcpEStatsPerfHCDataOctetsOut */
    uint64_t tcpi_bytes_retrans; /* RFC4898 tcpEStatsPerfOctetsRetrans */
    uint32_t tcpi_dsack_dups;    /* RFC4898 tcpEStatsStackDSACKDups */
    uint32_t tcpi_reord_seen;    /* reordering events seen */
};

/* netlink attributes types for SCM_TIMESTAMPING_OPT_STATS */
enum {
    _TCP_NLA_PAD,
    _TCP_NLA_BUSY,                  /* Time (usec) busy sending data */
    _TCP_NLA_RWND_LIMITED,          /* Time (usec) limited by receive window */
    _TCP_NLA_SNDBUF_LIMITED,        /* Time (usec) limited by send buffer */
    _TCP_NLA_DATA_SEGS_OUT,         /* Data pkts sent including retransmission */
    _TCP_NLA_TOTAL_RETRANS,         /* Data pkts retransmitted */
    _TCP_NLA_PACING_RATE,           /* Pacing rate in bytes per second */
    _TCP_NLA_DELIVERY_RATE,         /* Delivery rate in bytes per second */
    _TCP_NLA_SND_CWND,              /* Sending congestion window */
    _TCP_NLA_REORDERING,            /* Reordering metric */
    _TCP_NLA_MIN_RTT,               /* minimum RTT */
    _TCP_NLA_RECUR_RETRANS,         /* Recurring retransmits for the current pkt */
    _TCP_NLA_DELIVERY_RATE_APP_LMT, /* delivery rate application limited ? */
    _TCP_NLA_SNDQ_SIZE,             /* Data pending in send queue */
    _TCP_NLA_CA_STATE,              /* ca_state of socket */
    _TCP_NLA_SND_SSTHRESH,          /* Slow start size threshold */
    _TCP_NLA_DELIVERED,             /* Data pkts delivered incl. out-of-order */
    _TCP_NLA_DELIVERED_CE,          /* Like above but only ones w/ CE marks */
    _TCP_NLA_BYTES_SENT,            /* Data bytes sent including retransmission */
    _TCP_NLA_BYTES_RETRANS,         /* Data bytes retransmitted */
    _TCP_NLA_DSACK_DUPS,            /* DSACK blocks received */
    _TCP_NLA_REORD_SEEN,            /* reordering events seen */
    _TCP_NLA_SRTT,                  /* smoothed RTT in usecs */
};

/* TCP ca_state */
enum {
    _TCP_CA_Open,
    _TCP_CA_Disorder,
    _TCP_CA_CWR,
    _TCP_CA_Recovery,
    _TCP_CA_Loss,
};

#define TCP_INFINITE_SSTHRESH 0x7fffffff

enum {
    _SK_MEMINFO_RMEM_ALLOC,
    _SK_MEMINFO_RCVBUF,
    _SK_MEMINFO_WMEM_ALLOC,
    _SK_MEMINFO_SNDBUF,
    _SK_MEMINFO_FWD_ALLOC,
    _SK_MEMINFO_WMEM_QUEUED,
    _SK_MEMINFO_OPTMEM,
    _SK_MEMINFO_BACKLOG,
    _SK_MEMINFO_DROPS,

    _SK_MEMINFO_VARS,
};

/* INET_DIAG_VEGASINFO */

struct _tcpvegas_info {
    uint32_t tcpv_enabled;
    uint32_t tcpv_rttcnt;
    uint32_t tcpv_rtt;
    uint32_t tcpv_minrtt;
};

/* INET_DIAG_DCTCPINFO */

struct _tcp_dctcp_info {
    uint16_t dctcp_enabled;
    uint16_t dctcp_ce_state;
    uint32_t dctcp_alpha;
    uint32_t dctcp_ab_ecn;
    uint32_t dctcp_ab_tot;
};

/* INET_DIAG_BBRINFO */

struct _tcp_bbr_info {
    /* u64 bw: max-filtered BW (app throughput) estimate in Byte per sec: */
    uint32_t bbr_bw_lo;       /* lower 32 bits of bw */
    uint32_t bbr_bw_hi;       /* upper 32 bits of bw */
    uint32_t bbr_min_rtt;     /* min-filtered RTT in uSec */
    uint32_t bbr_pacing_gain; /* pacing gain shifted left 8 bits */
    uint32_t bbr_cwnd_gain;   /* cwnd gain shifted left 8 bits */
};

union _tcp_cc_info {
    struct _tcpvegas_info vegas;
    struct _tcp_dctcp_info dctcp;
    struct _tcp_bbr_info bbr;
};
#endif /* linux */

#if defined(__FreeBSD__)

/* Data returned by the TCP_INFO socket option on FreeBSD. */
struct _tcp_info {
    u_int8_t tcpi_state;
    u_int8_t __tcpi_ca_state;
    u_int8_t __tcpi_retransmits;
    u_int8_t __tcpi_probes;
    u_int8_t __tcpi_backoff;
    u_int8_t tcpi_options;
    u_int8_t tcpi_snd_wscale : 4, tcpi_rcv_wscale : 4;

    u_int32_t tcpi_rto;
    u_int32_t __tcpi_ato;
    u_int32_t tcpi_snd_mss;
    u_int32_t tcpi_rcv_mss;

    u_int32_t __tcpi_unacked;
    u_int32_t __tcpi_sacked;
    u_int32_t __tcpi_lost;
    u_int32_t __tcpi_retrans;
    u_int32_t __tcpi_fackets;

    u_int32_t __tcpi_last_data_sent;
    u_int32_t __tcpi_last_ack_sent;
    u_int32_t tcpi_last_data_recv;
    u_int32_t __tcpi_last_ack_recv;

    u_int32_t __tcpi_pmtu;
    u_int32_t __tcpi_rcv_ssthresh;
    u_int32_t tcpi_rtt;
    u_int32_t tcpi_rttvar;
    u_int32_t tcpi_snd_ssthresh;
    u_int32_t tcpi_snd_cwnd;
    u_int32_t __tcpi_advmss;
    u_int32_t __tcpi_reordering;

    u_int32_t __tcpi_rcv_rtt;
    u_int32_t tcpi_rcv_space;

    /* FreeBSD extensions to tcp_info. */
    u_int32_t tcpi_snd_wnd;
    u_int32_t tcpi_snd_bwnd;
    u_int32_t tcpi_snd_nxt;
    u_int32_t tcpi_rcv_nxt;
    u_int32_t tcpi_toe_tid;
    u_int32_t tcpi_snd_rexmitpack;
    u_int32_t tcpi_rcv_ooopack;
    u_int32_t tcpi_snd_zerowin;

    /* Padding to grow without breaking ABI. */
    u_int32_t __tcpi_pad[26]; /* Padding. */
};

#endif /* __FreeBSD__ */

#endif /* __TCP_HEADERS_H__ */