#!/usr/bin/python2
import sys

print sys.argv[0]
kern = open(sys.argv[1], "rb").read()
hook = open(sys.argv[2], "rb").read()
patch1 = open(sys.argv[3], "rb").read()
patch2 = open(sys.argv[4], "rb").read()
hashtag = open(sys.argv[5], "rb").read()

kern_len = len(kern)
kern_len_up = (kern_len + 0x10000) & ~0xFFFF;
for i in range(kern_len, kern_len_up):
    kern += "\0"

kern += hashtag
kern = kern[:0x6cc] + hook + kern[0x6cc+4:]

# buncha nops to bypass kernel mem checks
kern = kern[:0x1969C] + patch1[:4] + kern[0x1969C+4:]
kern = kern[:0x196B8] + patch1[:4] + kern[0x196B8+4:]
kern = kern[:0x196D4] + patch1[:4] + kern[0x196D4+4:]

# change norm mapping to 0xDC7E03
kern = kern[:0x19D98] + patch1[8:8+4] + kern[0x19D98+4:]
kern = kern[:0x19D9C] + patch1[12:12+4] + kern[0x19D9C+4:]

# norm mem is valid
kern = kern[:0x58058] + patch1[4:8] + kern[0x58058+4:]

# expand KSession obj size
kern = kern[:0x32544] + patch2[0:4] + kern[0x32544+4:]
kern = kern[:0x32548] + patch2[4:8] + kern[0x32548+4:]
kern = kern[:0x32588] + patch2[8:12] + kern[0x32588+4:]
kern = kern[:0x32598] + patch2[12:16] + kern[0x32598+4:]

# allow smcPanic, kprintf
kern = kern[:0x57F58] + patch1[:4] + kern[0x57F58+4:]
kern = kern[:0x57560] + patch1[:4] + kern[0x57560+4:]

open(sys.argv[6], "wb").write(kern)


