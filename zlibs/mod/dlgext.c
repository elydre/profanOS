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
    elfobj_t *libc = kmalloc(sizeof(elfobj_t));
    mem_cpy(libc, g_libc, sizeof(elfobj_t));
    libc->ref_count = 1;
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
    uint32_t size = obj->dym_size / sizeof(Elf32_Sym);

    dlg_hash_t *table = kcalloc(size, sizeof(dlg_hash_t));
    dlg_hash_t *later = kcalloc(size, sizeof(dlg_hash_t));
    int later_index = 0;

    for (uint32_t i = 0; i < size; i++) {
        const char *key = obj->dym_str + obj->dym_tab[i].st_name;
        uint32_t full_h = hash(key);
        uint32_t h = full_h % size;

        if (!table[h].data) {
            table[h].data = obj->dym_tab + i;
            table[h].key = key;
            table[h].hash = full_h;
        } else {
            later[later_index].data = obj->dym_tab + i;
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
    kfree(later);

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
        mem_cmp(ehdr->e_ident, (void *) ELFMAG, SELFMAG) != 0 ||
        ehdr->e_type != ET_DYN ||
        ehdr->e_machine != EM_386
    );
}

void *open_elf(char *filename) {
    elfobj_t *obj = kcalloc(1, sizeof(elfobj_t));

    uint32_t sid = fu_path_to_sid(SID_ROOT, filename);
    if (!fu_is_file(sid)) {
        raise_error("'%s' not found", filename);
        kfree(obj);
        return NULL;
    }

    obj->size = fu_file_get_size(sid);
    obj->file = kmalloc(obj->size);
    obj->name = filename;
    obj->ref_count = -1;

    fu_file_read(sid, obj->file, 0, obj->size);

    if (obj->size < sizeof(Elf32_Ehdr) || !is_valid_elf(obj->file)) {
        raise_error("'%s' is not a valid ELF file", filename);
        kfree(obj->file);
        kfree(obj);
        return NULL;
    }

    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)obj->file;
    Elf32_Shdr *shdr = (Elf32_Shdr *)(obj->file + ehdr->e_shoff);

    for (int i = 0; i < ehdr->e_shnum; i++) {
        switch (shdr[i].sh_type) {
            case 2: // SHT_SYMTAB
                obj->sym_tab = (Elf32_Sym *)(obj->file + shdr[i].sh_offset);
                obj->sym_str = (char *) obj->file + shdr[shdr[i].sh_link].sh_offset;
                obj->sym_size = shdr[i].sh_size;
                break;

            case 6: // SHT_STRTAB
                obj->dynamic = (Elf32_Dyn *)(obj->file + shdr[i].sh_offset);
                break;

            case 11: // SHT_DYNSYM
                obj->dym_tab = (Elf32_Sym *)(obj->file + shdr[i].sh_offset);
                obj->dym_str = (char *) obj->file + shdr[shdr[i].sh_link].sh_offset;
                obj->dym_size = shdr[i].sh_size;
                break;

            default:
                break;
        }
    }

    if (obj->dym_tab == NULL) {
        raise_error("no dynamic symbol table found in '%s'", filename);
        kfree(obj->file);
        kfree(obj);
        return NULL;
    }

    obj->hash_table = hash_create(obj);

    return obj;
}
