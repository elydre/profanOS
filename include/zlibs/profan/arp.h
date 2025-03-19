/*****************************************************************************\
|   === arp.h : 2025 ===                                                      |
|                                                                             |
|    Command line argument parsing library header file             .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef _PROFAN_ARP_H
#define _PROFAN_ARP_H

#define ARP_STANDARD 0
#define ARP_NEXT_STR 1
#define ARP_NEXT_INT 2
#define ARP_ANYNUMBR 3

#define ARP_FNOMAX 0x00ffffff
#define ARP_FMIN(min) ((min & 0xff) << 24)

int  arp_init(char *usage, unsigned file_limits);
void arp_set_ver(const char *name, const char *version);

int arp_register(char c, int flag, const char *desc);
int arp_conflict(const char *conflict);
int arp_parse(int argc, char **argv);

void arp_print_help(void);

int          arp_isset(char c);
const char  *arp_get_str(char c);
int          arp_get_int(char c);

const char **arp_get_files(void);
int          arp_file_count(void);
const char  *arp_file_next(void);

#endif
