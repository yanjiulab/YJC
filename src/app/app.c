/**
 * @file app.c
 * @author Yanjiu Li (liyanjiu@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-01-28
 *
 * @copyright Copyright (c) 2024
 *
 * BUGS:
 * 1. 退出时的处理，有卡顿。涉及到多线程和事件循环对信号的处理流程优化。
 */

#include "base.h"
#include "iniparser.h"
#include "log.h"
#include "sev.h"
#include "sniffer.h"
#include "thread.h"
#include "socket.h"
#include "sockopt.h"

// #include <net/if.h>

#define LIBCMDF_IMPL
#include "cmdf.h"
#define PROG_INTRO                                                             \
    "Welcome to YJC vtysh! YJC is a simple program for network programming.\n" \
    "You can use this as a reference on how to use the library!"
#define CMDF_BE_PORT "6688"

/* Global vars */
evloop_t* loop = NULL;
sniffer_t* sniff = NULL;
thread_t cmdf_tid;

static void sig_int() {
    cmdf__default_do_exit(NULL);
#ifdef CMDF_READLINE_SUPPORT
    rl_cleanup_after_signal();
    rl_free_line_state();
#endif
    evloop_stop(loop);
}

static CMDF_RETURN do_hello(cmdf_arglist* arglist) {
    printf("\nHello, world!\n");

    return CMDF_OK;
}

static CMDF_RETURN do_setlog(cmdf_arglist* arglist) {
    int level = 1;

    if (arglist && arglist->count) {
        level = str2int(arglist->args[0]) % 6;
    }
    log_set_level(level);

    return CMDF_OK;
}

static CMDF_RETURN do_quit(cmdf_arglist* arglist) {
    sig_int();

    return CMDF_OK;
}

// $ nc 127.0.0.1 CMDF_BE_PORT
THREAD_ROUTINE(backend_cmdf) {
    log_info("thread %lu start", thread_id());
    int fd = (int)userdata;
    FILE* f = fdopen(fd, "w+");
    cmdf_init("tcp> ", PROG_INTRO, NULL, NULL, 0, 1, f, f);
    cmdf_register_command(do_quit, "quit", "Quit the application");
    cmdf_register_command(do_setlog, "log", "Set log debug level. SYNOPSIS: log [level]");
    cmdf_commandloop();
    cmdf_quit;
    log_info("thread %lu exit", thread_id());
}

THREAD_ROUTINE(frontend_cmdf) {
    char* cmd = (char*)userdata;
    log_info("thread %lu start", thread_id());
    cmdf_init(str_fmt("%s> ", cmd), PROG_INTRO, NULL, NULL, 0, 0, 0, 0);
    // cmdf_init("ab >", PROG_INTRO, NULL, NULL, 0, 0, 0, 0);
    cmdf_register_command(do_quit, "quit", "Quit the application");
    cmdf_register_command(do_setlog, "log", "Set log debug level. SYNOPSIS: log [level]");
    cmdf_commandloop();
    log_info("thread %lu exit", thread_id());
}

static EV_RETURN on_period_hello(evtimer_t* timer) {
    // Do task
    log_info("Hello World %d", timer->id);

    // Re-register timer
    uint32_t to = (int)timer->data;
    evtimer_add(evloop(timer), on_period_hello, timer->data, to);

    return EV_OK;
}

static EV_RETURN on_accept(evio_t* io) {
    struct sockaddr_in cliaddr;
    socklen_t clilen = sizeof(cliaddr);
    int conn;
    conn = accept(io->fd, (struct sockaddr*)&cliaddr, &clilen);
    log_info("client [%s:%d] connected console.", inet_ntoa(cliaddr.sin_addr),
             ntohs(((struct sockaddr_in*)&cliaddr)->sin_port));

    cmdf_tid = thread_create(backend_cmdf, conn);

    return EV_OK;
}

static EV_RETURN on_udp(evio_t* io) {
    char recvline[1024] = {0};
    struct sockaddr_in cliaddr;
    socklen_t addrlen = sizeof(cliaddr);

    int n = recvfrom(io->fd, recvline, 1024, 0, (struct sockaddr*)&cliaddr, &addrlen);

    printf("client [%s:%d] recv %d bytes:\n", inet_ntoa(cliaddr.sin_addr),
           ntohs(((struct sockaddr_in*)&cliaddr)->sin_port), n);
    print_data(recvline, n);
    // int s = sendto(io->fd, recvline, n, 0, (struct sockaddr*)&cliaddr, sizeof(cliaddr));

    return EV_OK;
}

static EV_RETURN on_sniffer(evio_t* io) {
    char* error = NULL;
    char* dump = NULL;
    enum packet_parse_result_t result;

    int rcv_status = sniffer_recv(sniff);

    result = parse_packet(sniff->packet, sniff->packet_len, PACKET_LAYER_2_ETHERNET, &error);
    if (result != PACKET_OK) {
        printf("%s", error);
    }

    result = packet_stringify(sniff->packet, DUMP_FULL, &dump, &error);
    if (result != STATUS_OK) {
        printf("%s", error);
    }

    print_line('-', 0);
    printf("%s", dump);

    return 0;
}

int main(int argc, char* argv[]) {

    /* Parse command line args */
    int opt, isdaemon = 0;
    const char* usage = "Usage: %s [-h] [-d]\n";
    while ((opt = getopt(argc, argv, "hd")) != -1) {
        switch (opt) {
        case 'h':
            printf(usage, argv[0]);
            exit(0);
        case 'd':
            isdaemon = 1;
            break;
        default:
            fprintf(stderr, usage, argv[0]);
            exit(1);
        }
    }

    /* Parse ini config file */
    dictionary* ini = iniparser_load("config.ini");
    iniparser_dump(ini, stderr);
    // int i = iniparser_getint(ini, "sec:intkey", 0);
    // char *s = iniparser_getstring(ini, "sec:strkey", 0);
    // bool b = iniparser_getboolean(ini, "sec:boolkey", false);
    iniparser_freedict(ini);

    /* Get program name */
    char* cmd;
    if ((cmd = strrchr(argv[0], '/')) == NULL) {
        cmd = argv[0];
    } else {
        cmd++;
    }

    /* Check root privilege */
    uid_t uid = getuid();
    if (uid) {
        log_warn("Please run %s in root privilege!", cmd);
        return 0;
    }

    /* Setting log */
    int log_level = _LOG_DEBUG;
    log_set_level(log_level);
    log_info("Set term log level: %s", log_level_string(log_level));

    /* Program running in daemon mode */
    if (isdaemon) {
        log_info("%s running in daemon mode!", cmd);
        daemonize(cmd);
        FILE* logf = fopen("log/app.log", "w");
        log_add_fp(logf, log_level);
        log_info("Set file log level: %s", log_level_string(log_level));
    }

    /* Check if program is already running. */
    // if (already_running(cmd)) {
    //     // if (isdaemon)
    //     syslog(LOG_ERR, "%s was already running!", cmd);
    //     // else
    //     log_error("%s was already running!", cmd);
    //     exit(EXIT_FAILURE);
    // }

    /* Setting signals and clean */
    if (signal(SIGINT, sig_int) == SIG_ERR) {
        exit(EXIT_FAILURE);
    }

    /* Create an event loop */
    log_info("Create an event loop");
    loop = evloop_new(10);

    /* Add timer event */
    log_info("Add a timer event");
    evtimer_add(loop, on_period_hello, 5000, 5000);

    // UDP server
    int udp_fd = udp_server(ANYADDR, "520", NULL);
    struct in_addr group;
    struct in_addr ifaddr;
    group.s_addr = htonl(0xe1000009);
    so_bindtodev(udp_fd, "ens38");
    setsockopt_ipv4_multicast(udp_fd, IP_ADD_MEMBERSHIP, ifaddr, group.s_addr, 3);

    // struct sockaddr_in servaddr;
    // servaddr.sin_family = AF_INET;
    // servaddr.sin_addr.s_addr = inet_addr("192.168.0.114");
    // // servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    // servaddr.sin_port = htons(8866);

    // int totlen = msglen + scrp_hdr_len;

    // if (sendto(udp_fd, sendbuf, totlen, 0, (struct sockaddr*)&servaddr, sizeof(servaddr)) != (ssize_t)totlen) {
    //     log_warn("SEND %s message error (%s)", scrp_kind(type, subtype), strerror(errno));
    //     return -1;

    evio_add(loop, udp_fd, on_udp);

    // Sniffer
    sniff = sniffer_new(NULL);
    sniffer_set_record(sniff, SNIFFER_RECORD_PCAP, 100, NULL);
    sniffer_set_direction(sniff, DIRECTION_ALL);
    sniffer_set_filter_str(sniff, "arp");
    sniffer_start(sniff);
    // evio_add(loop, sniff->psock->packet_fd, on_sniffer);

    /* Start commandline loop */
    if (isdaemon) {
        log_info("Start cmd framework backend mode");
        int tcp_fd = tcp_listen(ANYADDR, CMDF_BE_PORT, NULL);
        evio_add(loop, tcp_fd, on_accept);
    } else {
        log_info("Start cmd framework frontend mode");
        cmdf_tid = thread_create(frontend_cmdf, cmd);
    }

    /* Run event loop */
    log_info("Run event loop");
    evloop_run(loop);

    /* Clean and exit */
    thread_join(cmdf_tid, NULL);
    log_fatal("Clean ...");
    log_info("...");
    log_fatal("Bye.");

    return 0;
}