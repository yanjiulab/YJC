#!/usr/bin/python

import os

src = 'src'
test = 'test'

funcs = []

def update_header():
    # create test.h
    with open('test/test.h', 'w') as f:
        f.write('#ifndef TEST_H\n')
        f.write('#define TEST_H\n')
        f.write('#include <assert.h>\n\n')
        for l in funcs:
            f.write(f'void {l}();\n')
        f.write('\n#endif  // !TEST_H\n')


def update_file():
    for sm in os.listdir(src):
        if sm == 'app':
            continue
        tm = os.path.join(test, sm)
        print(tm)
        for sf in os.listdir(os.path.join(src, sm)):
            if not sf.endswith('.h'):
                continue
            tf = os.path.join(test, 'test_' + sf.split('.')[0] + '.c')
            funcs.append('test_' + sf.split('.')[0])
            if os.path.exists(tf):
                print(f'file {tf} exists')
                continue
            else:
                print(f'create file {tf}')
                with open(tf, 'w') as f:
                    f.write(f'#include "test.h"\n')
                    f.write(f'#include "{sf}"\n\n')
                    f.write(f'void test_{sf.split(".")[0]}() {{}}\n')


if __name__ == "__main__":
    update_file()
    update_header()
