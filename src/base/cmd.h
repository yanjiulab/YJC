#ifndef _CMD_H_
#define _CMD_H_

#include "linenoise.h"
#include "log.h"
#include "base.h"
#include "array.h"

/* Error codes (for CMDF_RETURN) */
#define CMD_OK                        0
#define CMD_ERROR_TOO_MANY_COMMANDS   -1
#define CMD_ERROR_TOO_MANY_ARGS       -2
#define CMD_ERROR_UNKNOWN_COMMAND     -3
#define CMD_ERROR_ARGUMENT_ERROR      -4
#define CMD_ERROR_OUT_OF_MEMORY       -5
#define CMD_ERROR_DUPLICATED_COMMANDS -6

/* Flags */
#define CMD_FLAG_ASYNC                0x00000001

#define ASYNC_BUFFLEN                 1024
#define CMD_PPRINT_RIGHT_OFFSET       1
#define CMD_FREE                      free
#define CMD_MALLOC                    malloc

/* cmdf_windowsize struct */
struct cmd_windowsize {
    unsigned short h, w;
};

typedef struct cmd_ctx_s cmd_ctx_t;

typedef struct cmd_arglist_s {
    char** args;  /* NULL-terminated string list */
    size_t count; /* Argument list count */
} cmd_arglist_t;

typedef int (*cmd_command_cb)(cmd_ctx_t* ctx, cmd_arglist_t* arglist);

typedef struct cmd_entry_s {
    const char* cmdname;     /* Command name */
    const char* help;        /* Help */
    cmd_command_cb callback; /* Command callback */
} cmd_entry_t;

ARRAY_DECL(cmd_entry_t*, entry_array)

struct cmd_ctx_s {
    uint32_t flags;
    /* Properties */
    const char *prompt, *intro, *doc_header, *undoc_header;
    char ruler;

    /* Counters */
    int undoc_cmds, doc_cmds, entry_count;

    /* Index in cmd__entries array from which commands would be active */
    // int entry_start;
    entry_array entries;

    /* Flags */
    int exit_flag;

    /* in, out, err */
    FILE* cmd_stdin;
    FILE* cmd_stdout;
    // FILE* cmd_stderr;
    // int cmd_stdin;
    // int cmd_stdout;

    /* Callback pointers */
    cmd_command_cb do_emptyline;
    // int (*do_command)(cmd_ctx_t* ctx, const char*, cmd_arglist_t*);

    // Async
    struct linenoiseState ls;
    char* async_buff;
    bool async_ready;

    // uint32_t async_bufflen;
};

/* Argument Parsing */
cmd_arglist_t* cmd_parse_arguments(char* argline);
void cmd_free_arglist(cmd_arglist_t* arglist);

/* Default callbacks */
int cmd_default_do_command(cmd_ctx_t* ctx, const char* cmdname, cmd_arglist_t* arglist);
int cmd_default_do_help(cmd_ctx_t* ctx, cmd_arglist_t* arglist);
int cmd_default_do_emptyline(cmd_ctx_t* ctx, cmd_arglist_t* arglist /* Unused */);
int cmd_default_do_exit(cmd_ctx_t* ctx, cmd_arglist_t* arglist /* Unused */);
// int cmd_default_do_noop(cmd_arglist_t* arglist /* Unused */);
// int cmd_default_do_q(int, int);

/* Public interface functions */
/* Create Command context */
cmd_ctx_t* cmd_ctx_new(int flags, FILE* cmd_stdin, FILE* cmd_stdout, const char* prompt);
/* Adding/Removing Command Entries */
int cmd_register_command(cmd_ctx_t* ctx, cmd_command_cb callback, const char* cmdname, const char* help);
/* Synchronized API */
void cmd_commandloop(cmd_ctx_t* ctx);
/* Asynchronized API */
// in async mode, user should get input command string (by some eventloop)
// and then call it to process cmd.
int cmd_command_process(cmd_ctx_t* ctx, const char* inputbuff);

#endif