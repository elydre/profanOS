/****** This file is part of profanOS **************************\
|   == assert.c ==                                   .pi0iq.    |
|                                                   d"  . `'b   |
|   Implementation of assert internal function      q. /|\  u   |
|   for the C standard library                       `// \\     |
|                                                    //   \\    |
|   [ github.com/elydre/profanOS - GPLv3 ]          //     \\   |
\***************************************************************/

#include <stdlib.h>
#include <stdio.h>

void __assert_fail(const char *expr, const char *file, int line, const char *func) {
    fprintf(stderr, "Assertion failed: %s, file %s, line %d, function %s\n", expr, file, line, func);
    abort();
}
