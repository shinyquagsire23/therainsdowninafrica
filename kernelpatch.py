#!/usr/bin/python2
import sys

print sys.argv[0]
kern = open(sys.argv[1], "rb").read()
hook = open(sys.argv[2], "rb").read()
hashtag = open(sys.argv[3], "rb").read()

kern_len = len(kern)
kern_len_up = (kern_len + 0x10000) & ~0xFFFF;
for i in range(kern_len, kern_len_up):
    kern += "\0"

kern += hashtag
kern = kern[:0x6cc] + hook + kern[0x6cc+4:]

open(sys.argv[4], "wb").write(kern)


