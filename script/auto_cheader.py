#!/bin/python3

import re
import sys


def gen_func_decl():
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
            decl = func_return_type + ' ' + func_name + '(' + func_args + ');\n'
            f.write(decl)
            print(decl, end='')
    print(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>")

if __name__ == "__main__":
    gen_func_decl()
