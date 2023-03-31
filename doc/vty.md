# VTY


使用方式：

```c
/* Help display function for all node. */
DEFUN (config_list,
       config_list_cmd,
       "list",
       "Print command list\n")
{
  unsigned int i;
  struct cmd_node *cnode = vector_slot (cmdvec, vty->node);
  struct cmd_element *cmd;

  for (i = 0; i < vector_active (cnode->cmd_vector); i++)
    if ((cmd = vector_slot (cnode->cmd_vector, i)) != NULL
        && !(cmd->attr == CMD_ATTR_DEPRECATED
             || cmd->attr == CMD_ATTR_HIDDEN))
      vty_out (vty, "  %s%s", cmd->string,
	       VTY_NEWLINE);
  return CMD_SUCCESS;
}
```

其中，DEFUN 宏定义如下：

```c
#define DEFUN(funcname, cmdname, cmdstr, helpstr) \
  DEFUN_CMD_FUNC_DECL(funcname) \
  DEFUN_CMD_ELEMENT(funcname, cmdname, cmdstr, helpstr, 0, 0) \
  DEFUN_CMD_FUNC_TEXT(funcname)

#define DEFUN_CMD_ELEMENT(funcname, cmdname, cmdstr, helpstr, attrs, dnum) \
  struct cmd_element cmdname = \
  { \
    .string = cmdstr, \
    .func = funcname, \
    .doc = helpstr, \
    .attr = attrs, \
    .daemon = dnum, \
  };

#define DEFUN_CMD_FUNC_DECL(funcname) \
  static int funcname (struct cmd_element *, struct vty *, int, const char *[]);

#define DEFUN_CMD_FUNC_TEXT(funcname) \
  static int funcname \
    (struct cmd_element *self __attribute__ ((unused)), \
     struct vty *vty __attribute__ ((unused)), \
     int argc __attribute__ ((unused)), \
     const char *argv[] __attribute__ ((unused)) )
```

因此上述函数宏定义模板为

```c
DEFUN (config_list,
       config_list_cmd,
       "list",
       "Print command list\n")
{
    // ...
}

```

展开如下
```c
static int config_list (struct cmd_element *, struct vty *, int, const char *[]);
struct cmd_element config_list_cmd = {
    .string = "list", 
    .func = config_list, 
    .doc = "Print command list\n", 
    .attr = 0, 
    .daemon = 0, 
};
static int config_list(struct cmd_element *self, struct vty *vty, int argc , const char *argv[]) {
    unsigned int i;
    struct cmd_node *cnode = vector_slot(cmdvec, vty->node);
    struct cmd_element *cmd;

    for (i = 0; i < vector_active(cnode->cmd_vector); i++)
        if ((cmd = vector_slot(cnode->cmd_vector, i)) != NULL &&
            !(cmd->attr == CMD_ATTR_DEPRECATED || cmd->attr == CMD_ATTR_HIDDEN))
            vty_out(vty, "  %s%s", cmd->string, VTY_NEWLINE);
    return CMD_SUCCESS;
}

```


则展开结果为：
```c
static int funcname (struct cmd_element *, struct vty *, int, const char *[]);


```
DEFUN_CMD_FUNC_DECL(config_list) DEFUN_CMD_ELEMENT(config_list, config_list_cmd, "list", "Print command list\n", 0, 0) DEFUN_CMD_FUNC_TEXT(config_list)