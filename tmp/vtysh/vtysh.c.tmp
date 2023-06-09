#include "vtysh.h"

#include <readline/history.h>
#include <readline/readline.h>

/* A static variable for holding the line. */
static char *line_read;

/* Prototypes */
static char *vtysh_rl_gets();
static int vtysh_rl_describe(void);

char *vtysh_prompt(void) { return "[YJCSH]$ "; }

/* To disable readline's filename completion. */
static char *vtysh_completion_entry_function(const char *ignore, int invoking_key) { return NULL; }

/* We don't care about the point of the cursor when '?' is typed. */
static int vtysh_rl_describe(void) {
    // int ret;
    // unsigned int i;
    // vector vline;
    // vector describe;
    // int width;
    // struct cmd_token *token;

    // vline = cmd_make_strvec(rl_line_buffer);

    // /* In case of '> ?'. */
    // if (vline == NULL) {
    //     vline = vector_init(1);
    //     vector_set(vline, NULL);
    // } else if (rl_end && isspace((int)rl_line_buffer[rl_end - 1]))
    //     vector_set(vline, NULL);

    // describe = cmd_describe_command(vline, vty, &ret);

    // fprintf(stdout, "\n");

    // /* Ambiguous and no match error. */
    // switch (ret) {
    //     case CMD_ERR_AMBIGUOUS:
    //         cmd_free_strvec(vline);
    //         fprintf(stdout, "%% Ambiguous command.\n");
    //         rl_on_new_line();
    //         return 0;
    //         break;
    //     case CMD_ERR_NO_MATCH:
    //         cmd_free_strvec(vline);
    //         fprintf(stdout, "%% There is no matched command.\n");
    //         rl_on_new_line();
    //         return 0;
    //         break;
    // }

    // /* Get width of command string. */
    // width = 0;
    // for (i = 0; i < vector_active(describe); i++)
    //     if ((token = vector_slot(describe, i)) != NULL) {
    //         int len;

    //         if (token->cmd[0] == '\0') continue;

    //         len = strlen(token->cmd);
    //         if (token->cmd[0] == '.') len--;

    //         if (width < len) width = len;
    //     }

    // for (i = 0; i < vector_active(describe); i++)
    //     if ((token = vector_slot(describe, i)) != NULL) {
    //         if (token->cmd[0] == '\0') continue;

    //         if (!token->desc)
    //             fprintf(stdout, "  %-s\n", token->cmd[0] == '.' ? token->cmd + 1 : token->cmd);
    //         else
    //             fprintf(stdout, "  %-*s  %s\n", width, token->cmd[0] == '.' ? token->cmd + 1 : token->cmd,
    //             token->desc);
    //     }

    // cmd_free_strvec(vline);
    // vector_free(describe);

    // rl_on_new_line();
    printf("fuck");

    return 0;
}

static char **new_completion(char *text, int start, int end) {
    char **matches;

    // matches = rl_completion_matches(text, command_generator);

    // if (matches) {
    //     rl_point = rl_end;
    //     if (complete_status != CMD_COMPLETE_FULL_MATCH) /* only append a space on full match */
    //         rl_completion_append_character = '\0';
    // }

    return matches;
}

void vtysh_readline_init(void) {
    /* readline related settings. */
    rl_bind_key('?', (rl_command_func_t *)vtysh_rl_describe);
    rl_completion_entry_function = vtysh_completion_entry_function;
    rl_attempted_completion_function = (rl_completion_func_t *)new_completion;
}

/* Read a string, and return a pointer to it.  Returns NULL on EOF. */
static char *vtysh_rl_gets() {
    HIST_ENTRY *last;
    /* If the buffer has already been allocated, return the memory
     * to the free pool. */
    if (line_read) {
        free(line_read);
        line_read = NULL;
    }

    /* Get a line from the user.  Change prompt according to node.  XXX. */
    line_read = readline(vtysh_prompt());

    /* If the line has any text in it, save it on the history. But only if
     * last command in history isn't the same one. */
    if (line_read && *line_read) {
        using_history();
        last = previous_history();
        if (!last || strcmp(last->line, line_read) != 0) {
            add_history(line_read);
            // append_history(1, history_file);
        }
    }

    return (line_read);
}

/* Command execution over the vty interface. */
static int vtysh_execute_func(const char *line, int pager) {
    int ret, cmd_stat;
    u_int i;
    vector vline;
    struct cmd_element *cmd;
    FILE *fp = NULL;
    int closepager = 0;
    int tried = 0;
    int saved_ret, saved_node;

    /* Split readline string up into the vector. */
    vline = cmd_make_strvec(line);

    if (vline == NULL) return CMD_SUCCESS;

    saved_ret = ret = cmd_execute_command(vline, vty, &cmd, 1);
    saved_node = vty->node;

    /* If command doesn't succeeded in current node, try to walk up in node tree.
     * Changing vty->node is enough to try it just out without actual walkup in
     * the vtysh. */
    while (ret != CMD_SUCCESS && ret != CMD_SUCCESS_DAEMON && ret != CMD_WARNING && vty->node > CONFIG_NODE) {
        vty->node = node_parent(vty->node);
        ret = cmd_execute_command(vline, vty, &cmd, 1);
        tried++;
    }

    vty->node = saved_node;

    /* If command succeeded in any other node than current (tried > 0) we have
     * to move into node in the vtysh where it succeeded. */
    if (ret == CMD_SUCCESS || ret == CMD_SUCCESS_DAEMON || ret == CMD_WARNING) {
        if ((saved_node == BGP_VPNV4_NODE || saved_node == BGP_VPNV6_NODE || saved_node == BGP_ENCAP_NODE ||
             saved_node == BGP_ENCAPV6_NODE || saved_node == BGP_IPV4_NODE || saved_node == BGP_IPV6_NODE ||
             saved_node == BGP_IPV4M_NODE || saved_node == BGP_IPV6M_NODE) &&
            (tried == 1)) {
            vtysh_execute("exit-address-family");
        } else if ((saved_node == KEYCHAIN_KEY_NODE) && (tried == 1)) {
            vtysh_execute("exit");
        } else if (tried) {
            vtysh_execute("end");
            vtysh_execute("configure terminal");
        }
    }
    /* If command didn't succeed in any node, continue with return value from
     * first try. */
    else if (tried) {
        ret = saved_ret;
    }

    cmd_free_strvec(vline);

    cmd_stat = ret;
    switch (ret) {
        case CMD_WARNING:
            if (vty->type == VTY_FILE) fprintf(stdout, "Warning...\n");
            break;
        case CMD_ERR_AMBIGUOUS:
            fprintf(stdout, "%% Ambiguous command.\n");
            break;
        case CMD_ERR_NO_MATCH:
            fprintf(stdout, "%% Unknown command.\n");
            break;
        case CMD_ERR_INCOMPLETE:
            fprintf(stdout, "%% Command incomplete.\n");
            break;
        case CMD_SUCCESS_DAEMON: {
            /* FIXME: Don't open pager for exit commands. popen() causes problems
             * if exited from vtysh at all. This hack shouldn't cause any problem
             * but is really ugly. */
            if (pager && vtysh_pager_name && (strncmp(line, "exit", 4) != 0)) {
                fp = popen(vtysh_pager_name, "w");
                if (fp == NULL) {
                    perror("popen failed for pager");
                    fp = stdout;
                } else
                    closepager = 1;
            } else
                fp = stdout;

            if (!strcmp(cmd->string, "configure terminal")) {
                for (i = 0; i < array_size(vtysh_client); i++) {
                    cmd_stat = vtysh_client_execute(&vtysh_client[i], line, fp);
                    if (cmd_stat == CMD_WARNING) break;
                }

                if (cmd_stat) {
                    line = "end";
                    vline = cmd_make_strvec(line);

                    if (vline == NULL) {
                        if (pager && vtysh_pager_name && fp && closepager) {
                            if (pclose(fp) == -1) {
                                perror("pclose failed for pager");
                            }
                            fp = NULL;
                        }
                        return CMD_SUCCESS;
                    }

                    ret = cmd_execute_command(vline, vty, &cmd, 1);
                    cmd_free_strvec(vline);
                    if (ret != CMD_SUCCESS_DAEMON) break;
                } else if (cmd->func) {
                    (*cmd->func)(cmd, vty, 0, NULL);
                    break;
                }
            }

            cmd_stat = CMD_SUCCESS;
            for (i = 0; i < array_size(vtysh_client); i++) {
                if (cmd->daemon & vtysh_client[i].flag) {
                    cmd_stat = vtysh_client_execute(&vtysh_client[i], line, fp);
                    if (cmd_stat != CMD_SUCCESS) break;
                }
            }
            if (cmd_stat != CMD_SUCCESS) break;

            if (cmd->func) (*cmd->func)(cmd, vty, 0, NULL);
        }
    }
    if (pager && vtysh_pager_name && fp && closepager) {
        if (pclose(fp) == -1) {
            perror("pclose failed for pager");
        }
        fp = NULL;
    }
    return cmd_stat;
}

static int vtysh_execute(const char *line) { printf("shit\n"); }

/* VTY shell main routine. */
void shell() {
    /* Initialize user input buffer. */
    line_read = NULL;
    setlinebuf(stdout);

    /* Signal and others. */
    // vtysh_signal_init();

    /* Make vty structure and register commands. */
    // vtysh_init_vty();
    // vtysh_init_cmd();
    // vtysh_user_init();
    // vtysh_config_init();

    // vty_init_vtysh();

    /* Read vtysh configuration file before connecting to daemons. */
    // vtysh_read_config(config_default);

    /* Make sure we pass authentication before proceeding. */
    // vtysh_auth();

    /*
     * Setup history file for use by both -c and regular input
     * If we can't find the home directory, then don't store
     * the history information
     */
    // homedir = vtysh_get_home();
    // if (homedir) {
    //     snprintf(history_file, sizeof(history_file), "%s/.history_quagga", homedir);
    //     if (read_history(history_file) != 0) {
    //         int fp;

    //         fp = open(history_file, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
    //         if (fp) close(fp);

    //         read_history(history_file);
    //     }
    // }

    vtysh_readline_init();
    cmd_init();

    /* Main command loop. */
    while (vtysh_rl_gets()) {
        vtysh_execute(line_read);
    }

    /* Rest in peace. */
    exit(0);
}