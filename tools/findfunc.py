#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#
#   === findfunc.py : 2024 ===                                                #
#                                                                             #
#    Python script to get func name from addr and map file         .pi0iq.    #
#                                                                 d"  . `'b   #
#    This file is part of profanOS and is released under          q. /|\  "   #
#    the terms of the GNU General Public License                   `// \\     #
#                                                                  //   \\    #
#   === elydre : https://github.com/elydre/profanOS ===         #######  \\   #
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#

import sys

if len(sys.argv) < 2:
    print(f"Usage: {sys.argv[0]} <address>")
    sys.exit(1)

def findfunc(addr):
    # address is in hex and can be imprecise
    # if the addr is in the middle of two functions, the lower one is returned

    addr = int(addr, 16)
    old_name = None
    with open("out/make/kernel.map") as f:
        for line in f:
            l = line.split()
            if len(l) != 2 or l[0][0] != '0' or l[0][1] != 'x':
                continue
            laddr = int(l[0][2:], 16)
            if laddr > addr:
                return old_name
            old_name = l[1]
    return old_name

for e in sys.argv[1:]:
    print(e, findfunc(e))
