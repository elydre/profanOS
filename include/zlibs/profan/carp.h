/*****************************************************************************\
|   === carp.h : 2025 ===                                                     |
|                                                                             |
|    Command line argument parsing library header file             .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef _PROFAN_CARP_H
#define _PROFAN_CARP_H

#include <profan/minimal.h>

_BEGIN_C_FILE

#define CARP_STANDARD 0
#define CARP_NEXT_STR 1
#define CARP_NEXT_INT 2
#define CARP_ANYNUMBR 3

#define CARP_FNOMAX 0x00ffffff
#define CARP_FMIN(min) ((min & 0xff) << 24)

int  carp_init(char *usage, unsigned file_limits);
void carp_set_ver(const char *name, const char *version);

int carp_register(char c, int flag, const char *desc);
int carp_conflict(const char *conflict);
int carp_parse(int argc, char **argv);

void carp_print_help(void);

int          carp_isset(char c);
const char  *carp_get_str(char c);
int          carp_get_int(char c);

const char **carp_get_files(void);
int          carp_file_count(void);
const char  *carp_file_next(void);

_END_C_FILE

#endif
