#include "cmd.h"

static const char* cmd_default_prompt = "(libcmd) ";
static const char* cmd_default_intro = "Welcome to cmd";
static const char* cmd_default_doc_header = "Documented Commands:";
static const char* cmd_default_undoc_header = "Undocumented Commands:";
static const char cmd_default_ruler = '=';

/* Default callbacks */
int cmd_default_do_help(cmd_ctx_t* ctx, cmd_arglist_t* arglist) {
    int i;
    size_t offset;
    cmd_entry_t* entry;

    if (arglist) {
        if (arglist->count == 1) {
            for (i = 0; i < ctx->entry_count; i++) {
                if (strcmp(arglist->args[0], ctx->entries.ptr[i]->cmdname) == 0) {
                    /* Print help, if any */
                    if (ctx->entries.ptr[i]->help) {
                        printf("%s\n", ctx->entries.ptr[i]->help);
                    } else {
                        printf("\n(No documentation)\n");
                    }
                    return CMD_OK;
                }
            }
            /* If we reached this, means that the command was not found */
            fprintf(stdout, "Command '%s' was not found.\n", arglist->args[0]);
            return CMD_ERROR_UNKNOWN_COMMAND;
        } else {
            fprintf(stdout, "Too many arguments for the 'help' command!\n");
            return CMD_ERROR_TOO_MANY_ARGS;
        }
    } else {
        printf("doc cmd: \n");
        for (int i = 0; i < ctx->entry_count; i++) {
            entry = ctx->entries.ptr[i];
            if (entry->help) {
                printf("%s: %s\n", entry->cmdname, entry->help);
            }
        }

        printf("\nundoc cmd: \n");
        for (int i = 0; i < ctx->entry_count; i++) {
            entry = ctx->entries.ptr[i];
            if (!entry->help) {
                printf("%s\n", entry->cmdname);
            }
        }
    }

    return CMD_OK;

    /* If no arguments provided, print all help listing.
     * Otherwise, print documentation on specified command. */
    // if (arglist) {
    //     if (arglist->count == 1) {
    //         for (i = cmd_settings_stack.top->entry_start; i < cmd_settings_stack.top->entry_start + cmd_settings_stack.top->entry_count; i++) {
    //             if (strcmp(arglist->args[0], cmd_entries[i].cmdname) == 0) {
    //                 /* Print help, if any */
    //                 if (cmd_entries[i].help) {
    //                     offset = fprintf(CMDF_STDOUT, "%s   ", cmd_entries[i].cmdname);
    //                     cmd_pprint(offset, cmd_entries[i].help);
    //                 } else
    //                     printf("\n(No documentation)\n");

    //                 return CMD_OK;
    //             }
    //         }

    //         /* If we reached this, means that the command was not found */
    //         fprintf(CMDF_STDOUT, "Command '%s' was not found.\n", arglist->args[0]);
    //         return CMDF_ERROR_UNKNOWN_COMMAND;
    //     } else {
    //         fprintf(CMDF_STDOUT, "Too many arguments for the 'help' command!\n");
    //         return CMDF_ERROR_TOO_MANY_ARGS;
    //     }
    // } else
    //     cmd_print_command_list();

    // fputc('\n', CMDF_STDOUT);

    // return CMD_OK;
}

int cmd_default_do_emptyline(cmd_ctx_t* ctx, cmd_arglist_t* arglist /* Unusued */) {
    return CMD_OK;
}

int cmd_default_do_exit(cmd_ctx_t* ctx, cmd_arglist_t* arglist /* Unused */) {
    ctx->exit_flag = 1;

    return CMD_OK;
}

int cmd_default_do_noop(cmd_ctx_t* ctx, cmd_arglist_t* arglist /* Unused */) {
    return CMD_OK;
}

int cmd_default_do_q(int count, int key) {
    //     cmd_default_do_help(NULL);
    // #ifdef CMDF_READLINE_SUPPORT
    //     rl_on_new_line();
    // #endif
    return 0;
}

int cmd_default_do_command(cmd_ctx_t* ctx, const char* cmdname, cmd_arglist_t* arglist) {
    int i;

    /* Iterate through the commands list. Find and execute the appropriate command */
    for (i = 0; i < ctx->entry_count; i++) {
        if (strcmp(cmdname, ((cmd_entry_t*)ctx->entries.ptr[i])->cmdname) == 0)
            return ((cmd_entry_t*)ctx->entries.ptr[i])->callback(ctx, arglist);
    }

    return CMD_ERROR_UNKNOWN_COMMAND;
}

int cmd_do_mask(cmd_arglist_t* arglist) {
    linenoiseMaskModeEnable();
}

int cmd_do_unmask(cmd_arglist_t* arglist) {
    linenoiseMaskModeDisable();
}

/* Utility Functions */
static void cmd_trim(char* src) {
    char *begin = src, *end, *newln;
    size_t end_location;

    /* Check for empty string */
    if (strlen(src) == 1 && (src[0] == '\n' || src[0] == '\0')) {
        src[0] = '\0';
        return;
    }

    /* Replace newline */
    newln = strrchr(src, '\n');
    if (newln)
        *newln = '\0';

    /* Replace spaces and re-align the string */
    while (isspace((int)*begin))
        begin++;

    if (src != begin) {
        end_location = strlen(begin);
        memmove(src, begin, strlen(begin) + 1);
        memset(begin + end_location, '\0', sizeof(char) * strlen(begin - end_location));
    }

    /* Replaces spaces at the end of the string */
    end = strrchr(src, '\0');

    /* Check if we haven't reached the end of the string.
     * Because if we did, we have nothing else to do. */
    if (src == end)
        return;
    else
        end--;

    if (isspace((int)*end)) {
        do {
            *end = '\0';
            end--;
        } while (isspace((int)*end));
    }
}

static char* cmd_strdup(const char* src) {
    char* dst = (char*)(malloc(sizeof(char) * (strlen(src) + 1))); /* src + '\0' */
    if (!dst)
        return NULL;

    return strcpy(dst, src);
}

void cmd_default_completion(const char* buf, linenoiseCompletions* lc) {
    if (buf[0] == 'h') {
        linenoiseAddCompletion(lc, "help");
        linenoiseAddCompletion(lc, "help exit");
        linenoiseAddCompletion(lc, "help echo");
    }
}

char* cmd_default_hints(const char* buf, int* color, int* bold) {
    if (!strcasecmp(buf, "help")) {
        *color = 35;
        *bold = 0;
        return " world";
    }
    return NULL;
}

/* API */
cmd_ctx_t* cmd_ctx_new(int flags) {
    cmd_ctx_t* ctx;
    EV_ALLOC_SIZEOF(ctx);

    // entries
    entry_array_init(&ctx->entries, 16);

    ctx->prompt = cmd_default_prompt;
    ctx->intro = cmd_default_intro;
    ctx->doc_header = cmd_default_doc_header;
    ctx->undoc_header = cmd_default_undoc_header;
    ctx->ruler = cmd_default_ruler;
    ctx->doc_cmds = ctx->undoc_cmds = ctx->entry_count = 0;
    // ctx->cmd_stdin = stdin;
    // ctx->cmd_stdout = stdout;

    /* Set command callbacks */

    // ctx->do_command = cmd_default_do_command;
    ctx->do_emptyline = cmd_default_do_emptyline;
    ctx->flags |= flags; // if (flags & CMD_FLAG_ASYNC)

    // For test purpose
    cmd_register_command(ctx, cmd_do_mask, "/mask", "Enable mask mode.");
    cmd_register_command(ctx, cmd_do_unmask, "/unmask", "Disable mask mode.");

    /* Register exit callback, if required */
    // if (use_default_exit)
    cmd_register_command(ctx, cmd_default_do_exit, "exit", "Exit the cmd framework");
    cmd_register_command(ctx, cmd_default_do_exit, "exit", "Exit the cmd framework");
    /* Register help callback */
    cmd_register_command(ctx, cmd_default_do_help, "help",
                         "Get information on a command"
                         " or list commands.");

    /* Set the completion callback. This will be called every time the
     * user uses the <tab> key. */
    linenoiseSetCompletionCallback(cmd_default_completion);
    linenoiseSetHintsCallback(cmd_default_hints);
    linenoiseSetMultiLine(1);
    if (ctx->flags & CMD_FLAG_ASYNC) {
        // if (!ctx->async_buff)
        ctx->async_buff = malloc(ASYNC_BUFFLEN);
        char buff[1024];
        // linenoiseEditStart(&ctx->ls, -1, -1, ctx->async_buff, ASYNC_BUFFLEN, ctx->prompt);
        linenoiseEditStart(&ctx->ls, -1, -1, buff, 1024, ctx->prompt);
        // return ctx;
    }

    return ctx;
}

/* Adding/Removing Command Entries */
int cmd_register_command(cmd_ctx_t* ctx, cmd_command_cb callback, const char* cmdname,
                         const char* help) {

    /* Check cmdname */
    for (int i = 0; i < ctx->entry_count; i++) {
        if (strcmp(ctx->entries.ptr[i]->cmdname, cmdname) == 0) {
            return CMD_ERROR_DUPLICATED_COMMANDS;
        }
    }

    /* Initialize new entry */
    cmd_entry_t* entry;
    EV_ALLOC_SIZEOF(entry);
    entry->cmdname = cmdname;
    entry->help = help;
    entry->callback = callback;
    // entry->ctx = ctx;
    entry_array_push_back(&ctx->entries, &entry);
    ctx->entry_count++;

    /* Check doc */
    if (help)
        ctx->doc_cmds++;
    else
        ctx->undoc_cmds++;

    return CMD_OK;
}

void cmd_add_history(const char* line) {
#ifdef CMD_READLINE_SUPPORT
    add_history(inputbuff);
#endif
    linenoiseHistoryAdd(line); /* Add to the history. */
}

void cmd_save_history() {
    linenoiseHistorySave("history.txt"); /* Save the history on disk. */
}

cmd_arglist_t* cmd_parse_arguments(char* argline) {
    cmd_arglist_t* arglist = NULL;
    size_t i;
    char *strptr, *startptr;
    enum states { NONE,
                  IN_WORD,
                  IN_QUOTES } state = NONE;

    /* Check if there are any arguments */
    if (!argline)
        return NULL;

    /* Allocate argument list */
    arglist = (cmd_arglist_t*)(malloc(sizeof(cmd_arglist_t)));
    if (!arglist)
        return NULL;

    arglist->count = 0;

    /* First pass on the arguments line - use the state machine to determine how many
     * argument do we have. */
    for (strptr = startptr = argline, i = 0; *strptr; strptr++) {
        switch (state) {
        case NONE:
            /*
             * Space = Don't care.
             * Quotes = a quoted argument begins.
             * Anything else = Inside a word.
             */
            if (isspace((int)*strptr))
                continue;
            else if (*strptr == '\"')
                state = IN_QUOTES;
            else
                state = IN_WORD;

            break;
        case IN_QUOTES:
            /*
             * Space = Don't care, since we're inside quotes.
             * Quotes = Quotes have ended, so count++
             * Anything else = Don't care, since we're inside quotes.
             */
            if (isspace((int)*strptr))
                continue;
            else if (*strptr == '\"') {
                state = NONE;
                arglist->count++;

                break;
            } else
                continue;
        case IN_WORD:
            /*
             * Space = A word just terminated, so count++
             * Quotes = Ignore - quote is part of the word
             * Anything else = Still in word
             */
            if (isspace((int)*strptr)) {
                state = NONE;
                arglist->count++;

                break;
            } else if (*strptr == '\"')
                continue;
            else
                continue;
        }
    }

    /* Handle last argument counting, if any */
    if (state != NONE)
        arglist->count++;

    /* Now we can allocate the argument list */
    arglist->args = (char**)(malloc(sizeof(char*) * (arglist->count + 1))); /* + NULL */
    if (!arglist->args) {
        free(arglist);
        return NULL;
    }

    /* Populate argument list */
    state = NONE;
    for (strptr = startptr = argline, i = 0; *strptr; strptr++) {
        switch (state) {
        case NONE:
            /* Space = No word yet.
             * Quotes = Quotes started, so ignore everything inbetween.
             * Else = Probably a word. */
            if (isspace((int)*strptr))
                continue;
            else if (*strptr == '\"') {
                state = IN_QUOTES;
                startptr = strptr + 1;
            } else {
                state = IN_WORD;
                startptr = strptr;
            }

            break;
        case IN_QUOTES:
            /* Space = Ignore it, iterate futher.
             * Quotes = End quotes, so put the entire quoted part as an argument.
             * Else = Whatever is between the quotes */
            if (*strptr == '\"') {
                *strptr = '\0';
                arglist->args[i++] = cmd_strdup(startptr);
                state = NONE;
            }

            break;
        case IN_WORD:
            /* Space = End of word, so parse it.
             * Quote = Some quote inside of a word. We treat it is a word.
             * Else = Still a word. */
            if (isspace((int)*strptr)) {
                *strptr = '\0';
                arglist->args[i++] = cmd_strdup(startptr);
                state = NONE;
            }

            break;
        }
    }

    /* Get the last argument, if any. */
    if (state != NONE && i < arglist->count)
        arglist->args[i++] = cmd_strdup(startptr);

    /* Set up null terminator in the argument list */
    arglist->args[i] = NULL;

    return arglist;
}

void cmd_free_arglist(cmd_arglist_t* arglist) {
    size_t i;

    /* Check if any argument list was provided. */
    if (arglist) {
        /* Free every argument */
        for (i = 0; i < arglist->count - 1; i++)
            free(arglist->args[i]);

        free(arglist->args);
    }

    free(arglist);
}

char* cmd_async_commandloop(cmd_ctx_t* ctx) {
    char* inputbuff;
    char *cmdptr, *argsptr, *spcptr;
    cmd_arglist_t* cmd_args;
    int retflag;

    inputbuff = linenoiseEditFeed(&ctx->ls);
    /* A NULL return means: line editing is continuing.
     * Otherwise the user hit enter or stopped editing
     * (CTRL+C/D). */

    // if (inputbuff != linenoiseEditMore) return;

    // /* Trim string */
    // cmd_trim(inputbuff);

    // /* If input is empty, call do_emptyline command. */
    // // Never reach?
    // if (inputbuff[0] == '\0') {
    //     ctx->do_emptyline(ctx, NULL);
    //     return;
    // }

    // /* If we've reached this, then line has something in it.
    //  * If readline is enabled, save this to history. */
    // cmd_add_history(inputbuff);

    // /* Split by first space.
    //  * This should be the command, followed by arguments. */
    // if ((spcptr = strchr(inputbuff, ' '))) {
    //     *spcptr = '\0';

    //     cmdptr = inputbuff;
    //     argsptr = spcptr + 1;
    // } else {
    //     cmdptr = inputbuff;
    //     argsptr = NULL;
    // }

    // /* Parse arguments */
    // cmd_args = cmd_parse_arguments(argsptr);

    // /* Execute command. */
    // retflag = cmd_default_do_command(ctx, cmdptr, cmd_args);
    // switch (retflag) {
    // case CMD_ERROR_UNKNOWN_COMMAND:
    //     fprintf(stdout, "Unknown command '%s'.\n", cmdptr);
    //     break;
    // }
    // return inputbuff;
}

void cmd_commandloop(cmd_ctx_t* ctx) {
    // #ifndef CMDF_READLINE_SUPPORT
    //     char inputbuff[CMDF_MAX_INPUT_BUFFER_LENGTH];
    // #else
    char* inputbuff;
    // #endif

    char *cmdptr, *argsptr, *spcptr;
    cmd_arglist_t* cmd_args;
    int retflag;

    /* Print intro, if any. */
    if (ctx->intro)
        fprintf(stdout, "\n%s\n\n", ctx->intro);

    while (!ctx->exit_flag) {

        // #ifndef CMDF_READLINE_SUPPORT
        //         fprintf(CMDF_STDOUT, "%s", cmd_settings_stack.top->prompt);
        //         fgets(inputbuff, sizeof(char) * CMDF_MAX_INPUT_BUFFER_LENGTH, CMDF_STDIN);

        //         /* Check for EOF */
        //         if (feof(CMDF_STDIN)) {
        //             cmd_settings_stack.top->exit_flag = 1;
        //             continue;
        //         }
        // #endif

        /* Print prompt and get input */
        // inputbuff = readline(cmd_settings_stack.top->prompt);
        if (!(ctx->flags & CMD_FLAG_ASYNC)) {
            // running in blocking mode
            inputbuff = linenoise(ctx->prompt);
            /* EOF, or failure to allocate a buffer. Means we probably need to exit. */
            if (!inputbuff) {
                ctx->exit_flag = 1;
                continue;
            }
        } else {
            if (!ctx->async_buff)
                ctx->async_buff = malloc(ASYNC_BUFFLEN);
            linenoiseEditStart(&ctx->ls, -1, -1, ctx->async_buff, ASYNC_BUFFLEN, ctx->prompt);
            return;
        }

        /* Trim string */
        cmd_trim(inputbuff);

        /* If input is empty, call do_emptyline command. */
        if (inputbuff[0] == '\0') {
            ctx->do_emptyline(ctx, NULL);
            continue;
        }

        /* If we've reached this, then line has something in it.
         * If readline is enabled, save this to history. */
        cmd_add_history(inputbuff);

        /* Split by first space.
         * This should be the command, followed by arguments. */
        if ((spcptr = strchr(inputbuff, ' '))) {
            *spcptr = '\0';

            cmdptr = inputbuff;
            argsptr = spcptr + 1;
        } else {
            cmdptr = inputbuff;
            argsptr = NULL;
        }

        /* Parse arguments */
        cmd_args = cmd_parse_arguments(argsptr);

        /* Execute command. */
        retflag = cmd_default_do_command(ctx, cmdptr, cmd_args);
        switch (retflag) {
        case CMD_ERROR_UNKNOWN_COMMAND:
            fprintf(stdout, "Unknown command '%s'.\n", cmdptr);
            break;
        }

        /* Free arguments */
        cmd_free_arglist(cmd_args);

        // #ifdef CMDF_READLINE_SUPPORT
        // /* Free buffer */
        // free(inputbuff);
        // #endif
        free(inputbuff);
    }
}