/*****************************************************************************\
|   === cap.h : 2025 ===                                                      |
|                                                                             |
|    Command line argument parsing library header file             .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef _PROFAN_CAP_H
#define _PROFAN_CAP_H

#define CAP_STANDARD 0
#define CAP_NEXT_STR 1
#define CAP_NEXT_INT 2
#define CAP_ANYNUMBR 3

#define CAP_FNOMAX 0x00ffffff
#define CAP_FMIN(min) ((min & 0xff) << 24)

int  cap_init(char *usage, unsigned file_limits);
void cap_set_ver(const char *name, const char *version);

int cap_register(char c, int flag, const char *desc);
int cap_conflict(const char *conflict);
int cap_parse(int argc, char **argv);

void cap_print_help(void);

int          cap_isset(char c);
const char  *cap_get_str(char c);
int          cap_get_int(char c);

const char **cap_get_files(void);
int          cap_file_count(void);
const char  *cap_file_next(void);

#endif
