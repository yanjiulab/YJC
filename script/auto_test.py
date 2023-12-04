#!/usr/bin/python

import os

src = 'src'
test = 'test'

funcs = []

def update_test_h():
    # create test.h
    with open('test/test.h', 'w') as f:
        f.write('#ifndef TEST_H\n')
        f.write('#define TEST_H\n')
        f.write('#include <assert.h>\n\n')
        for l in funcs:
            f.write(f'void {l}();\n')
        f.write('\n#endif  // !TEST_H\n')


#
# create test file
for sm in os.listdir(src):
    if sm == 'app':
        continue
    tm = os.path.join(test, sm)
    print(tm)
    for sf in os.listdir(os.path.join(src, sm)):
        if not sf.endswith('.h'):
            continue
        tf = os.path.join(test, sm, sf)
        if os.path.exists(tf):
            print(f'{tf} exists')
            continue
        else:
            # name = 'test_' + sf.split('.')[0] + '.c'
            name = os.path.join(test, 'test_' + sf.split('.')[0] + '.c')
            funcs.append('test_' + sf.split('.')[0])
            # print(name)
            # with open(name, 'w') as f:
            #     f.write(f'#include "test.h"\n')
            #     f.write(f'#include "{sf}"\n\n')
            #     f.write(f'void test_{sf.split(".")[0]}() {{}}\n')


if __name__ == "__main__":
    pass
    # update_test_h()
