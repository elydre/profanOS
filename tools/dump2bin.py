#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#
#   === dump2bin.py : 2025 ===                                                #
#                                                                             #
#    Python script to restore hexdum -C output to binary file      .pi0iq.    #
#                                                                 d"  . `'b   #
#    This file is part of profanOS and is released under          q. /|\  "   #
#    the terms of the GNU General Public License                   `// \\     #
#                                                                  //   \\    #
#   === elydre : https://github.com/elydre/profanOS ===         #######  \\   #
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#

import sys

if len(sys.argv) != 3:
    print("Usage: python3 convert.py <input> <output>")
    sys.exit(1)

outfile = open(sys.argv[2], "wb")

with open(sys.argv[1], "r") as f:
    for line in f:
        try:
            data = bytes([int(x, 16) for x in line[10:58].split()])
        except:
            print("Error: Invalid input file")
            sys.exit(1)
        outfile.write(data)

outfile.close()
