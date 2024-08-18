/*****************************************************************************\
|   === dlgext.c : 2024 ===                                                   |
|                                                                             |
|    Dynamic linker extension to load libC only once               .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#define DLGEXT_C

#include <profan/filesys.h>
#include <profan/libmmq.h>
#include <profan/dlgext.h>

/****************************
 *                         *
 *    Types and globals    *
 *                         *
****************************/

#define raise_error(fmt, ...)  \
            fd_printf(2, "DLGEXT FATAL: "fmt"\n", ##__VA_ARGS__);

elfobj_t *g_libc;

/****************************
 *                         *
 *    Module main funcs    *
 *                         *
****************************/

void *open_elf(char *filename);

int main(void) {
    g_libc = open_elf("/lib/libc.so");
    if (!g_libc)
        return 1;
    return 0;
}

elfobj_t *dlgext_libc(void) {
    // duplicate libc object
    elfobj_t *libc = malloc(sizeof(elfobj_t));
    memcpy(libc, g_libc, sizeof(elfobj_t));
    libc->ref_count = 0;
    return libc;
}

/***************************
 *                        *
 *    Hash table funcs    *
 *                        *
***************************/

uint32_t hash(const char *str) {
    uint32_t hash = 0;
    for (int i = 0; str[i]; i++) {
        hash = (hash << 5) + str[i];
    }
    return hash;
}

dlg_hash_t *hash_create(elfobj_t *obj) {
    uint32_t size = obj->dynsym_size / sizeof(Elf32_Sym);

    dlg_hash_t *table = calloc(size, sizeof(dlg_hash_t));
    dlg_hash_t *later = calloc(size, sizeof(dlg_hash_t));
    int later_index = 0;

    for (uint32_t i = 0; i < size; i++) {
        const char *key = obj->dynstr + obj->dymsym[i].st_name;
        uint32_t full_h = hash(key);
        uint32_t h = full_h % size;

        if (!table[h].data) {
            table[h].data = obj->dymsym + i;
            table[h].key = key;
            table[h].hash = full_h;
        } else {
            later[later_index].data = obj->dymsym + i;
            later[later_index].key = key;
            later[later_index].hash = full_h;
            later_index++;
        }
    }

    uint32_t table_index = 0;
    for (int i = 0; i < later_index; i++) {
        uint32_t h = later[i].hash % size;
        dlg_hash_t *entry = &table[h];

        while (table[table_index].data) {
            if (++table_index == size) {
                raise_error("Internal error: hash table is full");
            }
        }

        table[table_index] = later[i];

        while (entry->next)
            entry = (void *) entry->next;
        entry->next = (void *) &table[table_index];

        table_index++;
    }
    free(later);

    return table;
}

/************************
 *                     *
 *    ELF functions    *
 *                     *
************************/

int is_valid_elf(void *data) {
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)data;
    return !(
        memcmp(ehdr->e_ident, (void *) ELFMAG, SELFMAG) != 0 ||
        ehdr->e_type != ET_DYN ||
        ehdr->e_machine != EM_386
    );
}

void *open_elf(char *filename) {
    elfobj_t *obj = calloc(1, sizeof(elfobj_t));

    sid_t sid = fu_path_to_sid(ROOT_SID, filename);
    if (!fu_is_file(sid)) {
        raise_error("'%s' not found", filename);
        free(obj);
        return NULL;
    }

    obj->size = fu_get_file_size(sid);
    obj->file = malloc(obj->size);
    obj->name = filename;
    obj->ref_count = -1;

    fu_file_read(sid, obj->file, 0, obj->size);

    if (obj->size < sizeof(Elf32_Ehdr) || !is_valid_elf(obj->file)) {
        raise_error("'%s' is not a valid ELF file", filename);
        free(obj->file);
        free(obj);
        return NULL;
    }

    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)obj->file;
    Elf32_Shdr *shdr = (Elf32_Shdr *)(obj->file + ehdr->e_shoff);

    for (int i = 0; i < ehdr->e_shnum; i++) {
        if (shdr[i].sh_type == 11) { // SHT_DYNSYM
            obj->dymsym = (Elf32_Sym *)(obj->file + shdr[i].sh_offset);
            obj->dynstr = (char *) obj->file + shdr[shdr[i].sh_link].sh_offset;
            obj->dynsym_size = shdr[i].sh_size;
        }

        if (shdr[i].sh_type == 6) { // SHT_DYNAMIC
            obj->dynamic = (Elf32_Dyn *)(obj->file + shdr[i].sh_offset);
        }
    }

    if (obj->dymsym == NULL) {
        raise_error("no dynamic symbol table found in '%s'", filename);
        free(obj->file);
        free(obj);
        return NULL;
    }

    obj->sym_table = hash_create(obj);

    return obj;
}

#undef SYSCALL_H
#define _SYSCALL_CREATE_FUNCS
#include <profan/syscall.h>
