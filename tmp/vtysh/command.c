#include "command.h"

/* Command vector which includes some level of command lists. */
vector(struct cmd_node *) *cmdvec = NULL;

static struct cmd_node view_node = {
    VIEW_NODE,
    "%s> ",
};

/* Install top node of command vector. */
void install_node(struct cmd_node *node, int (*func)(struct vty *)) {
    vec_insert(cmdvec, node->node, node);
    node->cmd_vector = vec_new(struct cmd_node *);

    // vector_set_index(cmdvec, node->node, node);
    // node->func = func;
    // node->cmd_vector = vector_init(VECTOR_MIN_SIZE);
    // node->cmd_hash = hash_create(cmd_hash_key, cmd_hash_cmp);
}

/* Install a command into a node. */
void install_element(enum node_type ntype, struct cmd_element *cmd) {
    struct cmd_node *cnode;

    cnode = vec_get(cmdvec, ntype);

    if (cnode == NULL) {
        fprintf(stderr, "Command node %d doesn't exist, please check it\n", ntype);
        exit(1);
    }

    vec_set(cnode->cmd_vector, &cmd);

    // if (cmd->tokens == NULL) {
    //     cmd->tokens = cmd_parse_format(cmd->string, cmd->doc);
    // }
}

/* Show version. */
DEFUN(show_version, show_version_cmd, "show version", SHOW_STR "Displays zebra version\n") {
    printf("show\n");
    return 0;
}

/* Initialize command interface. Install basic nodes and commands. */
void cmd_init() {
    cmdvec = vec_new(struct cmd_node *);

    /* Install top nodes. */
    install_node(&view_node, NULL);

    install_element(VIEW_NODE, &show_version_cmd);

    vec_dump(cmdvec, "%p");
}

/**
 * Execute a given command, handling things like "do ..." and checking
 * whether the given command might apply at a parent node if doesn't
 * apply for the current node.
 *
 * @param vline Command line input, vector of char* where each element is
 *              one input token.
 * @param vty The vty context in which the command should be executed.
 * @param cmd Pointer where the struct cmd_element of the matched command
 *            will be stored, if any. May be set to NULL if this info is
 *            not needed.
 * @param vtysh If set != 0, don't lookup the command at parent nodes.
 * @return The status of the command that has been executed or an error code
 *         as to why no command could be executed.
 */
// int cmd_execute_command(vector vline, struct vty *vty, struct cmd_element **cmd, int vtysh) {
//     int ret, saved_ret, tried = 0;
//     enum node_type onode, try_node;

//     onode = try_node = vty->node;

//     if (cmd_try_do_shortcut(vty->node, vector_slot(vline, 0))) {
//         vector shifted_vline;
//         unsigned int index;

//         vty->node = ENABLE_NODE;
//         /* We can try it on enable node, cos' the vty is authenticated */

//         shifted_vline = vector_init(vector_count(vline));
//         /* use memcpy? */
//         for (index = 1; index < vector_active(vline); index++) {
//             vector_set_index(shifted_vline, index - 1, vector_lookup(vline, index));
//         }

//         ret = cmd_execute_command_real(shifted_vline, FILTER_RELAXED, vty, cmd);

//         vector_free(shifted_vline);
//         vty->node = onode;
//         return ret;
//     }

//     saved_ret = ret = cmd_execute_command_real(vline, FILTER_RELAXED, vty, cmd);

//     if (vtysh) return saved_ret;

//     /* This assumes all nodes above CONFIG_NODE are childs of CONFIG_NODE */
//     while (ret != CMD_SUCCESS && ret != CMD_WARNING && vty->node > CONFIG_NODE) {
//         try_node = node_parent(try_node);
//         vty->node = try_node;
//         ret = cmd_execute_command_real(vline, FILTER_RELAXED, vty, cmd);
//         tried = 1;
//         if (ret == CMD_SUCCESS || ret == CMD_WARNING) {
//             /* succesfull command, leave the node as is */
//             return ret;
//         }
//     }
//     /* no command succeeded, reset the vty to the original node and
//        return the error for this node */
//     if (tried) vty->node = onode;
//     return saved_ret;
// }