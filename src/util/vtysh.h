#ifndef VTYSH_H_
#define VTYSH_H_

#include <stdio.h>
#include <stdlib.h>

#include "array.h"

//-----------------------------command----------------------------------------------
ARRAY_DECL(void *, tokens);
ARRAY_DECL(struct cmd_node *, cmd_vector);

/* There are some command levels which called from command node. */
enum node_type {
    VIEW_NODE,   /* View node. Default mode of vty interface. */
    CONFIG_NODE, /* Config node. Default mode of config file. */
};

/* Node which has some commands and prompt string and configuration
   function pointer . */
struct cmd_node {
    /* Node index. */
    enum node_type node;

    /* Prompt character at vty interface. */
    const char *prompt;

    /* Vector of this node's command list. */
    cmd_vector cmd_vector;
};

/* Structure of command element. */
struct cmd_element {
    const char *string; /* Command specification by string. */
    int (*func)(struct cmd_element *, int, const char *[]);
    const char *doc;       /* Documentation of this command. */
    int daemon;            /* Daemon to which this command belong. */
    // vector(void *) tokens; /* Vector of cmd_tokens */
    u_char attr;           /* Command attributes */
};

/* helper defines for end-user DEFUN* macros */
#define DEFUN_CMD_FUNC_DECL(funcname) static int funcname(struct cmd_element *, struct vty *, int, const char *[]);

#define DEFUN_CMD_ELEMENT(funcname, cmdname, cmdstr, helpstr, attrs, dnum) \
    struct cmd_element cmdname = {                                         \
        .string = cmdstr,                                                  \
        .func = funcname,                                                  \
        .doc = helpstr,                                                    \
        .attr = attrs,                                                     \
        .daemon = dnum,                                                    \
    };

#define DEFUN_CMD_FUNC_TEXT(funcname)                                                                              \
    static int funcname(struct cmd_element *self __attribute__((unused)), struct vty *vty __attribute__((unused)), \
                        int argc __attribute__((unused)), const char *argv[] __attribute__((unused)))

#define DEFUN(funcname, cmdname, cmdstr, helpstr)               \
    DEFUN_CMD_FUNC_DECL(funcname)                               \
    DEFUN_CMD_ELEMENT(funcname, cmdname, cmdstr, helpstr, 0, 0) \
    DEFUN_CMD_FUNC_TEXT(funcname)

/* Common descriptions. */
#define SHOW_STR "Show running system information\n"

/* Shell */
void shell();

#endif  // !VTYSH_H_