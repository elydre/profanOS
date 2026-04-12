/*****************************************************************************\
|   === getopt.h : 2026 ===                                                   |
|                                                                             |
|    Implementation of the getopt.h header file from libC          .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <profan/minimal.h>
#include <sys/cdefs.h>

_BEGIN_C_FILE

#ifndef _BASIC_GETOPT_H
#define _BASIC_GETOPT_H

// this part is also included from unistd.h

#ifdef __GNUC__
  extern int optind, opterr, optopt;
  extern char *optarg;
#else
  // old compilers make simple R_386_COPY
  // of external variables

  extern int   *__getoptind(void);
  extern int   *__getopterr(void);
  extern int   *__getoptopt(void);
  extern char **__getoptarg(void);

  #define optind (*__getoptind())
  #define opterr (*__getopterr())
  #define optopt (*__getoptopt())
  #define optarg (*__getoptarg())
#endif

int getopt(int argc, char *const *argv, const char *optstring);

#endif // _BASIC_GETOPT_H


#ifndef _LONG_GETOPT_H
#define _LONG_GETOPT_H

// this part is available only from getopt.h

#ifdef __GNUC__
  extern int optreset;
#else
  extern int *__getoptreset(void);
  #define optreset (*__getoptreset())
#endif

#define no_argument        0
#define required_argument  1
#define optional_argument  2

struct option {
    const char *name;   // name of long option
    int  has_arg;       // no_argument, required_argument or optional_argument
    int *flag;          // if not NULL, set *flag to val when option found
    int  val;           // if flag, value to set *flag to; else return value
};

int getopt_long(int argc, char *const *argv, const char *optstring,
                        const struct option *longopts, int *idx);

int getopt_long_only(int argc, char *const *argv, const char *optstring,
                        const struct option *longopts, int *idx);

#endif // _LONG_GETOPT_H

_END_C_FILE
