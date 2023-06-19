#!/bin/python3

import re
import sys


def gen_header_temp(filename):
    """
    生成一个头文件模板
    """
    n = str.upper(filename)
    s = '''\
/* %s.h */

#ifndef __%s_H__
#define __%s_H__

/* Headers */

/* Macros */

#ifdef __cplusplus
extern "C" {
#endif

/* Struct/Enum/Type */

/* Global variables */
// extern int global_var;

/* Prototypes */
// extern int foo(int, double);

#ifdef __cplusplus
}
#endif

/* Static functions */

#endif /* __%s_H__ */
    ''' % (filename, n, n, n)
    print(s)


def gen_func_decl():
    """
    根据 .c 文件中函数定义生成头文件函数声明
    """
    if len(sys.argv) != 2:
        print("invalid args")
        exit()
    filename = sys.argv[1]
    print("filename: " + filename)

    with open(filename, 'r') as f:
        content = f.read()

    pattern = r'^\s*([\w\s*]+)\**\s+(\w+)\((.*)\)\s*\{'
    matches = re.findall(pattern, content, re.MULTILINE)

    headername = filename.split('.')[0] + '.h'
    print("header: " + headername)
    print(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>")
    with open(headername, 'w') as f:
        for match in matches:
            func_return_type, func_name, func_args = match[0], match[1], match[2]
            decl = func_return_type + ' ' + \
                func_name + '(' + func_args + ');\n'
            f.write(decl)
            print(decl, end='')
    print(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>")


if __name__ == "__main__":
    gen_header_temp(sys.argv[1])
    # gen_func_decl()
