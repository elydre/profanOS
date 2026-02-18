/*****************************************************************************\
|   === main.c : 2026 ===                                                     |
|                                                                             |
|    -                                                             .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

extern int sumit(int a, int b);

int supercool_function() {
    return 42;
}

void *__profan_module_func[] = {
    (void *) 0xF3A3C4D4, // magic
    (void *) sumit,
    (void *) supercool_function,
};
