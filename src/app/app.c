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
#include "term.h"

#define LIBCMDF_IMPL
#define CMDF_READLINE_SUPPORT
#include "cmdf.h"
#define PROG_INTRO                                \
    "A simple program for network programming.\n" \
    "You can use this as a reference on how to use the library!"

/* Global vars */
evloop_t* loop = NULL;
sniffer_t* sniff = NULL;

static void sig_int() {
    cmdf__default_do_exit(NULL);
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

static EV_RETURN on_period_hello(evtimer_t* timer) {
    // Do task
    log_info("Hello World %d", timer->id);

    // Re-register timer
    uint32_t to = (int)timer->data;
    evtimer_add(evloop(timer), on_period_hello, timer->data, to);

    return EV_OK;
}

THREAD_ROUTINE(backend_cmdf) {
    log_info("thread %lu start", thread_id());
    int fd = (int)userdata;
    FILE* f = fdopen(fd, "w+");
    rl_instream = f;
    rl_outstream = f;
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

static EV_RETURN on_accept(evio_t* io) {
    struct sockaddr_in cliaddr;
    socklen_t clilen = sizeof(cliaddr);
    int conn;
    conn = accept(io->fd, (struct sockaddr*)&cliaddr, &clilen);
    log_info("client [%s:%d] connected console.", inet_ntoa(cliaddr.sin_addr),
             ntohs(((struct sockaddr_in*)&cliaddr)->sin_port));

    thread_create(backend_cmdf, conn);

    return EV_OK;
}

static EV_RETURN on_udp(evio_t* io) {
    char recvline[1024] = {0};
    struct sockaddr_in cliaddr;
    socklen_t addrlen = sizeof(cliaddr);

    int n = recvfrom(io->fd, recvline, 1024, 0, &cliaddr, &addrlen);

    printf("client [%s:%d] recv %d bytes:\n", inet_ntoa(cliaddr.sin_addr),
           ntohs(((struct sockaddr_in*)&cliaddr)->sin_port), n);
    print_data(recvline, n);
    int s = sendto(io->fd, recvline, n, 0, (struct sockaddr*)&cliaddr, sizeof(cliaddr));

    return EV_OK;
}

static void on_sniffer(evio_t* io) {
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
    if (already_running(cmd)) {
        // if (isdaemon)
        syslog(LOG_ERR, "%s was already running!", cmd);
        // else
        log_error("%s was already running!", cmd);
        exit(EXIT_FAILURE);
    }

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

    // UDP
    int udp_fd;
    struct sockaddr_in addr;
    udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(6688);
    bind(udp_fd, (struct sockaddr*)&addr, sizeof(addr));
    evio_add(loop, udp_fd, on_udp);

    // Sniffer
    sniff = sniffer_new(NULL);
    sniffer_set_record(sniff, SNIFFER_RECORD_PCAP, 100, NULL);
    sniffer_set_direction(sniff, DIRECTION_ALL);
    sniffer_set_filter_str(sniff, "arp");
    sniffer_start(sniff);
    evio_add(loop, sniff->psock->packet_fd, on_sniffer);

    /* Start commandline loop */
    if (isdaemon) {
        log_info("Start cmd framework backend mode");
        int tcp_fd;
        tcp_fd = socket(AF_INET, SOCK_STREAM, 0);
        bind(tcp_fd, (struct sockaddr*)&addr, sizeof(addr));
        listen(tcp_fd, 5);
        int on = 1;
        setsockopt(tcp_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(int));
        evio_add(loop, tcp_fd, on_accept);
    } else {
        log_info("Start cmd framework frontend mode");
        thread_create(frontend_cmdf, cmd);
    }

    /* Run event loop */
    log_info("Run event loop");
    evloop_run(loop);

    /* Clean and exit */
    log_fatal("Clean ...");
    log_info("...");
    log_fatal("Bye.");

    return 0;
}