/*****************************************************************************\
|   === dlgext.h : 2024 ===                                                   |
|                                                                             |
|    Header for the deluge linker and its extension                .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef DLGEXT_ID
#define DLGEXT_ID 1007

#include <elf.h>

/******************************
 *                           *
 *    Types and ext funcs    *
 *                           *
 *****************************/

typedef struct {
    const char *key;
    Elf32_Sym *data;
    uint32_t hash;
    void *next;
} dlg_hash_t;

typedef struct elfobj {
    char    *name;
    uint16_t type;

    int ref_count;

    uint8_t *mem;

    Elf32_Ehdr *ehdr;
    Elf32_Shdr *shdr;

    Elf32_Sym *sym_tab;
    uint32_t   sym_size;
    char      *sym_str;

    Elf32_Sym *dym_tab;
    uint32_t   dym_size;
    char      *dym_str;

    Elf32_Dyn  *dynamic;

    dlg_hash_t *hash_table;
} elfobj_t;

#ifndef DLGEXT_C
#define get_func_addr ((uint32_t (*)(uint32_t, uint32_t)) *(uint32_t *) 0x1ffffb)
#define dlgext_libc ((elfobj_t *(*)(void)) get_func_addr(DLGEXT_ID, 2))
#endif

#endif
