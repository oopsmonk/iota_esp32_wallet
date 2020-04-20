#!/usr/bin/env python
from __future__ import print_function # support python 2.x
import fileinput
import shutil
from os.path import join, realpath
import os

# get current path
os.chdir('iota_common/utils/containers/hash')

# cp and replace the SIZE 
for hash_size in [27, 81, 243, 6561, 8019]:
    src_file = "hash"+str(hash_size)+"_stack.c"
    hdr_file = "hash"+str(hash_size)+"_stack.h"
    shutil.copyfile('hash_stack.c.tpl', src_file)
    shutil.copyfile('hash_stack.h.tpl', hdr_file)

    q_src_file = "hash"+str(hash_size)+"_queue.c"
    q_hdr_file = "hash"+str(hash_size)+"_queue.h"
    shutil.copyfile('hash_queue.c.tpl', q_src_file)
    shutil.copyfile('hash_queue.h.tpl', q_hdr_file)
    # replace
    for line in fileinput.input([src_file, hdr_file, q_src_file, q_hdr_file], inplace=True):
        print(line.replace('{SIZE}', str(hash_size)), end='')