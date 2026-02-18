/*****************************************************************************\
|   === deluge.c : 2024 ===                                                   |
|                                                                             |
|    profanOS dynamic linker, load and run ELF                     .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

// @LINK: libmmq

#include <profan/syscall.h>
#include <modules/filesys.h>
#include <profan/libmmq.h>

#include <dlfcn.h>
#include <elf.h>

#define DELUGE_VERSION  "5.4"
#define ALWAYS_DEBUG    0

#define VISIBILITY_LOCAL  0
#define VISIBILITY_GLOBAL 1
#define VISIBILITY_INIT   2

/****************************
 *                         *
 *    Types and globals    *
 *                         *
****************************/

// hash table structure
typedef struct {
    const char *key;
    Elf32_Sym  *data;
    uint32_t    hash;
    void       *next;
} dlg_hash_t;

// ELF object structure
typedef struct elfobj {
    char    *name;
    uint16_t type;
    char     visibility;

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
    struct elfobj **deps;
} elfobj_t;

// extra symbols structure
typedef struct {
    const char *name;
    uint32_t    hash;
    void       *func;
} dlg_extra_t;

// internal variables
elfobj_t **g_loaded_libs;
elfobj_t  *g_prog;

int    g_dlfcn_error;
int    g_lib_count;
char **g_envp;

// extra symbols table
char *dlg_fn_name(void *ptr, char **libname);
void *dlg_open(const char *filename, int flag);
int   dlg_close(void *handle);
void *dlg_sym(void *handle, const char *symbol);
char *dlg_error(void);

dlg_extra_t g_extra_syms[] = {
    { "dlg_fn_name", 0xF5D604CF, dlg_fn_name },
    { "dlg_open"   , 0x36354BED, dlg_open    },
    { "dlg_close"  , 0xFC039D91, dlg_close   },
    { "dlg_sym"    , 0x18EA69D4, dlg_sym     },
    { "dlg_error"  , 0xFC2B2525, dlg_error   },
    { NULL         , 0         , NULL        },
};

// deluge arguments structure
typedef struct {
    char   *name;
    char   *extra_path;
    char   *extra_lib;
    char   *patch_file;
    uint8_t show_leaks;
    int     arg_offset;
    int     debug;
} deluge_args_t;

deluge_args_t *g_args;

/********************************
 *                             *
 *   Error and debug Defines   *
 *                             *
********************************/

#define raise_error(fmt, ...) {  \
        mmq_printf(2, "DELUGE FATAL: "fmt"\n", ##__VA_ARGS__); \
        mmq_exit(1);  \
    }

#define debug_printf(lvl, ...) {          \
    if (g_args->debug >= lvl) {           \
        mmq_putstr(2, "\e[37m[DELUGE] ");  \
        mmq_printf(2, __VA_ARGS__);        \
        mmq_putstr(2, "\e[0m\n");          \
    }}

#define debug_printf_tab(lvl, r, ...) {   \
    if (g_args->debug >= lvl) {           \
        mmq_putstr(2, "\e[37m[DELUGE] ");  \
        for (int i = 0; i < r - 1; i++)   \
            mmq_putstr(2, "--");           \
        if (r > 0)                        \
            mmq_putstr(2, "> ");           \
        mmq_printf(2, __VA_ARGS__);        \
        mmq_putstr(2, "\e[0m\n");          \
    }}

/********************************
 *                             *
 *      Utility functions      *
 *                             *
********************************/

void loaded_lib_add(elfobj_t *lib) {
    g_loaded_libs = mmq_realloc(g_loaded_libs, ++g_lib_count * sizeof(elfobj_t *));
    g_loaded_libs[g_lib_count - 1] = lib;
}

int loaded_lib_atback(elfobj_t *lib) {
    for (int i = 0; i < g_lib_count; i++) {
        if (g_loaded_libs[i] != lib)
            continue;
        for (int j = i; j < g_lib_count - 1; j++) {
            g_loaded_libs[j] = g_loaded_libs[j + 1];
        }
        g_loaded_libs[g_lib_count - 1] = lib;
        return 0;
    }
    return 1;
}

void *mem_dup(void *src, uint32_t size) {
    void *dst = mmq_malloc(size);
    mmq_memcpy(dst, src, size);
    return dst;
}

char *ft_getenv(const char *name) {
    uint32_t len = mmq_strlen(name);
    for (int i = 0; g_envp[i]; i++) {
        if (mmq_strncmp(g_envp[i], name, len) == 0 && g_envp[i][len] == '=')
            return g_envp[i] + len + 1;
    }
    return NULL;
}

char *profan_path_join(const char *dir, const char *file) {
    int len1 = mmq_strlen(dir);
    char *path = mmq_malloc(len1 + mmq_strlen(file) + 2);
    mmq_strcpy(path, dir);
    path[len1] = '/';
    mmq_strcpy(path + len1 + 1, file);
    return path;
}

/********************************
 *                             *
 *    File search functions    *
 *                             *
********************************/

uint32_t search_inpath(const char *src_path, const char *filename, char **fullpath) {
    char *path = mmq_strdup(src_path);

    char *fullname = mmq_malloc(mmq_strlen(filename) + 5); // 5 => .elf + null
    mmq_strcpy(fullname, filename);
    mmq_strcat(fullname, ".elf");

    int start = 0;
    for (int i = 0;; i++) {
        if (path[i] != ':' && path[i] != '\0')
            continue;
        path[i] = '\0';
        uint32_t sid = fu_path_to_sid(SID_ROOT, path + start);
        if (!IS_SID_NULL(sid)) {
            sid = fu_path_to_sid(sid, fullname);
            if (fu_is_file(sid)) {
                if (path)
                    *fullpath = profan_path_join(path + start, fullname);
                mmq_free(fullname);
                mmq_free(path);
                return sid;
            }
        }
        if (src_path[i] == '\0')
            break;
        start = i + 1;
    }

    mmq_free(fullname);
    mmq_free(path);
    return SID_NULL;
}

uint32_t search_elf_sid(const char *name, int islib, char **path) {
    char *full_path;
    uint32_t sid;

    if (name == NULL)
        return SID_NULL;

    if (name[0] == '/') {
        sid = fu_path_to_sid(SID_ROOT, name);
        if (!IS_SID_NULL(sid) && path)
            *path = mmq_strdup(name);
        return sid;
    }

    if (name[0] == '.' && name[1] == '/') {
        char *cwd = ft_getenv("PWD");
        if (!cwd)
            return SID_NULL;
        full_path = profan_path_join(cwd, name + 2);
        sid = fu_path_to_sid(SID_ROOT, full_path);
        if (!IS_SID_NULL(sid) && path)
            *path = full_path;
        else
            mmq_free(full_path);
        return sid;
    }

    if (!islib) {
        char *env_path = ft_getenv("PATH");
        if (!env_path)
            return SID_NULL;
        return search_inpath(env_path, name, path);
    }

    if (g_args->extra_path) {
        full_path = profan_path_join(g_args->extra_path, name);
        sid = fu_path_to_sid(SID_ROOT, full_path);
        if (!IS_SID_NULL(sid)) {
            if (path)
                *path = full_path;
            return sid;
        }
        mmq_free(full_path);
    }

    full_path = profan_path_join("/lib", name);
    sid = fu_path_to_sid(SID_ROOT, full_path);

    if (!IS_SID_NULL(sid)) {
        if (path)
            *path = full_path;
        return sid;
    }

    mmq_free(full_path);
    return SID_NULL;
}

/********************************
 *                             *
 *    Hash table functions     *
 *                             *
********************************/

static inline uint32_t hash(const char *str) {
    uint32_t hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;

    return hash;
}

dlg_hash_t *hash_create(elfobj_t *obj) {
    uint32_t size = obj->dym_size / sizeof(Elf32_Sym);

    dlg_hash_t *table = mmq_calloc(size, sizeof(dlg_hash_t));
    dlg_hash_t *later = mmq_calloc(size, sizeof(dlg_hash_t));
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

        table[table_index].next = entry->next;
        entry->next = &table[table_index];

        table_index++;
    }
    mmq_free(later);

    return table;
}

Elf32_Sym *hash_get(elfobj_t *obj, uint32_t full_h, const char *key) {
    dlg_hash_t *entry = obj->hash_table + full_h % (obj->dym_size / sizeof(Elf32_Sym));;

    while (entry) {
        if (entry->hash == full_h && mmq_strcmp(entry->key, key) == 0)
            return entry->data;
        entry = entry->next;
    }
    return NULL;
}

/********************************
 *                             *
 *     ELF check functions     *
 *                             *
********************************/

int is_valid_elf(void *data, uint16_t required_type) {
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)data;
    return !(
        mmq_memcmp(ehdr->e_ident, (void *) ELFMAG, SELFMAG) != 0 ||
        (required_type && ehdr->e_type != required_type) ||
        ehdr->e_machine != EM_386
    );
}

/********************************
 *                             *
 *    ELF loading functions    *
 *                             *
********************************/

void *get_base_addr(Elf32_Ehdr *ehdr, Elf32_Phdr *phdr) {
    // find the lowest address of a PT_LOAD segment

    uint32_t base_addr = 0xFFFFFFFF;

    for (int i = 0; i < ehdr->e_phnum; i++) {
        if (phdr[i].p_type == 1 && phdr[i].p_vaddr < base_addr)
            base_addr = phdr[i].p_vaddr;
    }

    return (void *) base_addr;
}

int load_sections(elfobj_t *obj, uint8_t *file) {
    Elf32_Shdr *shdr = (Elf32_Shdr *)(file + obj->ehdr->e_shoff);
    Elf32_Phdr *phdr = (Elf32_Phdr *)(file + obj->ehdr->e_phoff);

    void *base_addr = get_base_addr(obj->ehdr, phdr);
    void *offset = NULL;

    if (base_addr && obj->type != ET_EXEC) {
        raise_error("%s: NULL base address expected (got 0x%x)", obj->name, base_addr);
    }

    uint32_t required_size = 0;

    for (int i = 0; i < obj->ehdr->e_shnum; i++) {
        if (shdr[i].sh_addr && shdr[i].sh_addr + shdr[i].sh_size > required_size)
            required_size = shdr[i].sh_addr + shdr[i].sh_size;
    }

    if (obj->type == ET_EXEC)
        required_size -= (uint32_t) base_addr;
    required_size = (required_size + 0xFFF) & ~0xFFF;

    if (obj->type == ET_EXEC) {
        obj->mem = (void *) base_addr;
        (void) syscall_scuba_generate(base_addr, required_size / 0x1000);
    } else { // TODO: use a page
        obj->mem = (void *) syscall_mem_alloc(required_size, 1, 0x1000);
        offset = obj->mem;
    }

    for (int i = 0; i < obj->ehdr->e_shnum; i++) {
        if (!shdr[i].sh_addr)
            continue;
        switch (shdr[i].sh_type) {
            case SHT_NOBITS:
                mmq_memset(offset + shdr[i].sh_addr, 0, shdr[i].sh_size);
                break;
            default:
                mmq_memcpy(offset + shdr[i].sh_addr, file + shdr[i].sh_offset, shdr[i].sh_size);
                break;
        }
    }

    return 0;
}

/********************************
 *                             *
 *  ELF Relocation functions   *
 *                             *
********************************/

uint32_t search_sym_value(const char *name, elfobj_t *obj, int allow_global, int allow_local) {
    uint32_t full_h = hash(name);
    Elf32_Sym *s;

    // search in the extra symbols
    for (int i = 0; g_extra_syms[i].name; i++) {
        if (g_extra_syms[i].hash == full_h && mmq_strcmp(g_extra_syms[i].name, name) == 0)
            return (uint32_t) g_extra_syms[i].func;
    }

    // search in the object
    if (allow_local && obj && (s = hash_get(obj, full_h, name)) && s->st_shndx != STB_LOCAL) {
        if (obj->type == ET_EXEC)
            return s->st_value;
        return s->st_value + (uint32_t) obj->mem;
    }

    // search in the dependencies
    if (obj && obj->deps) for (int i = 0; obj->deps[i]; i++) {
        if ((s = hash_get(obj->deps[i], full_h, name)) && s->st_shndx != STB_LOCAL) {
            return s->st_value + (uint32_t) obj->deps[i]->mem;
        }
    }

    if (!allow_global)
        return 0;

    // search in the main program
    if (g_prog && (s = hash_get(g_prog, full_h, name)) && s->st_shndx != STB_LOCAL) {
        if (g_prog->type == ET_EXEC)
            return s->st_value;
        return s->st_value + (uint32_t) g_prog->mem;
    }

    // search in the global libraries
    for (int k = 0; k < g_lib_count; k++) {
        if (g_loaded_libs[k]->visibility == VISIBILITY_GLOBAL &&
            (s = hash_get(g_loaded_libs[k], full_h, name))    &&
            s->st_shndx != STB_LOCAL
        ) return s->st_value + (uint32_t) g_loaded_libs[k]->mem;
    }

    return 0;
}

char *get_addr_name(elfobj_t *obj, uint32_t addr) {
    if (!obj->sym_tab)
        return NULL;

    uint32_t offset = obj->type == ET_DYN ? (uint32_t) obj->mem : 0;

    for (uint32_t i = 0; i < obj->sym_size / sizeof(Elf32_Sym); i++) {
        uint32_t val = offset + obj->sym_tab[i].st_value;
        if (addr < val || addr >= val + obj->sym_tab[i].st_size)
            continue;
        return obj->sym_str + obj->sym_tab[i].st_name;
    }

    return NULL;
}

/* == 32-bit x86 relocations types ==
 *
 * R_386_NONE
 * R_386_32         S + A
 * R_386_PC32       S + A - P
 * R_386_GOT32      G + A
 * R_386_PLT32      L + A - P
 * R_386_COPY       S (symbol size)
 * R_386_GLOB_DAT   S
 * R_386_JMP_SLOT   S
 * R_386_RELATIVE   B + A
 * R_386_GOTOFF     S + A - GOT
 * R_386_GOTPC      GOT + A - P
 * R_386_32PLT      L + A

 * A:   addend
 * B:   base address of the shared object
 * GOT: address of the global offset table
 * L:   address of the procedure linkage table
 * P:   place of the storage unit being relocated
 * S:   value of the symbol
*/

static inline int does_type_required_sym(uint8_t type) {
    switch (type) {
        case R_386_32:
        case R_386_PC32:
        case R_386_COPY:
        case R_386_GLOB_DAT:
        case R_386_JMP_SLOT:
        case R_386_GOTOFF:
            return 1;
        default:
            return 0;
    }
}

int dynamic_linker(elfobj_t *obj) {
    if (obj->dym_tab == NULL)
        return 0;

    uint32_t *ptr;

    for (uint32_t i = 0; i < obj->ehdr->e_shnum; i++) {
        if (obj->shdr[i].sh_type == SHT_RELA)
            raise_error("SHT_RELA is not supported but found in '%s'", obj->name);

        if (obj->shdr[i].sh_type != SHT_REL)
            continue;

        Elf32_Rel *rel = (Elf32_Rel *)(obj->mem + obj->shdr[i].sh_offset);

        for (uint32_t j = 0; j < obj->shdr[i].sh_size / sizeof(Elf32_Rel); j++) {
            Elf32_Sym *sym = obj->dym_tab + ELF32_R_SYM(rel[j].r_info);

            uint8_t    type = ELF32_R_TYPE(rel[j].r_info);
            char      *name = obj->dym_str + sym->st_name;
            uint32_t   val = sym->st_value;

            if (does_type_required_sym(type)) {
                // check if the symbol is defined in the object
                if (val && type != R_386_COPY) {
                    if (obj->type == ET_DYN)
                        val += (uint32_t) obj->mem;
                } else {
                    val = search_sym_value(name, obj,
                            1, // allow global libraries
                            0  // don't check local symbols
                    );
                    if (val == 0)
                        raise_error("%s: symbol '%s' not found", obj->name, name);
                }
            }

            if (obj->type == ET_DYN)
                ptr = (uint32_t *)(obj->mem + rel[j].r_offset);
            else
                ptr = (uint32_t *)(rel[j].r_offset);

            switch (type) {
                case R_386_32:          // word32  S + A
                    *ptr = val + *ptr;
                    break;
                case R_386_PC32:        // word32  S + A - P
                    *ptr = val + *ptr - (uint32_t) ptr;
                    break;
                case R_386_COPY:        // symbol  S
                    mmq_memcpy(ptr, (void *) val, (obj->dym_tab + ELF32_R_SYM(rel[j].r_info))->st_size);
                    break;
                case R_386_RELATIVE:    // word32  B + A
                    if (obj->type != ET_EXEC)
                        *ptr = (uint32_t) obj->mem + *ptr;
                    break;
                case R_386_JMP_SLOT:    // word32  S
                case R_386_GLOB_DAT:    // word32  S
                    *ptr = val;
                    break;
                default:
                    raise_error("%s: relocation type %d in not supported", obj->name, type);
                    break;
            }
        }
    }
    return 0;
}

/********************************
 *                             *
 *   ELF init/fini functions   *
 *                             *
********************************/

void free_elf(elfobj_t *obj) {
    if (obj->type == ET_DYN)
        mmq_free(obj->mem);
    mmq_free(obj->hash_table);
    mmq_free(obj->sym_tab);
    mmq_free(obj->sym_str);
    mmq_free(obj->ehdr);
    mmq_free(obj->shdr);
    mmq_free(obj->name);
    mmq_free(obj->deps);
    mmq_free(obj);
}

#define open_elf(name, type, fatal, visibility) open_elf_r(name, type, fatal, visibility, 0)
#define elfobj_iof(obj, fini) elfobj_iof_r(obj, fini, 0)
#define close_elf(obj) close_elf_r(obj, 0)

void elfobj_iof_r(elfobj_t *obj, int fini, int lvl) {
    // call constructors or destructors

    if (obj->dynamic == NULL)
        return;

    void (**func_array)(void) = NULL;
    int size = 0;

    for (int j = 0; obj->dynamic[j].d_tag != 0; j++) {
        if (obj->dynamic[j].d_tag == (fini ? DT_FINI_ARRAY : DT_INIT_ARRAY)) {
            func_array = (void (**)(void)) obj->dynamic[j].d_un.d_ptr;
        }
        if (obj->dynamic[j].d_tag == (fini ? DT_FINI_ARRAYSZ : DT_INIT_ARRAYSZ)) {
            size = obj->dynamic[j].d_un.d_val / sizeof(void *);
        }
    }

    if (func_array == NULL)
        return;

    if (obj->type == ET_DYN)
        func_array = (void (**)(void)) ((uint32_t) func_array + (uint32_t) obj->mem);

    for (int i = 0; i < size; i++) {
        if (g_args->debug > 1) {
            char *name = get_addr_name(obj, (uint32_t) func_array[i]);
            if (name) {
                debug_printf_tab(2, lvl, "call %s %s()", obj->name, name);
            } else {
                debug_printf_tab(2, lvl, "call %s [%p]", obj->name, func_array[i]);
            }
        }
        func_array[i]();
    }
}

/********************************
 *                             *
 *    ELF opening functions    *
 *                             *
********************************/

void free_elf(elfobj_t *obj);

void *open_elf_r(const char *filename, uint16_t required_type, int isfatal, char visibility, int rlvl) {
    // 'rlvl' is the recursion level, used for debug output
    // set to -1 to avoid loading library dependencies

    char *path = NULL;
    uint32_t sid, size;
    uint8_t *file;

    sid = search_elf_sid(filename, required_type == ET_DYN, &path);

    if (IS_SID_NULL(sid)) {
        if (isfatal)
            raise_error("%s: file not found", filename);
        return NULL;
    }

    if (required_type == ET_DYN) {
        for (int i = 0; i < g_lib_count; i++) {
            if (mmq_strcmp(g_loaded_libs[i]->name, path))
                continue;
            debug_printf_tab(1, rlvl, "find %s", path);
            mmq_free(path);
            g_loaded_libs[i]->ref_count++;
            return g_loaded_libs[i];
        }
    }

    debug_printf_tab(1, rlvl, "open %s", path);

    size = fu_file_get_size(sid);
    file = mmq_malloc(size);

    fu_file_read(sid, file, 0, size);

    if (size < sizeof(Elf32_Ehdr) || !is_valid_elf(file, required_type)) {
        if (isfatal)
            raise_error("%s: invalid ELF file", path);
        mmq_free(file);
        return NULL;
    }

    elfobj_t *obj = mmq_calloc(1, sizeof(elfobj_t));

    obj->ref_count = 1;
    obj->name = path;
    obj->visibility = visibility == VISIBILITY_LOCAL ? VISIBILITY_LOCAL : VISIBILITY_GLOBAL;

    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)file;
    Elf32_Shdr *shdr = (Elf32_Shdr *)(file + ehdr->e_shoff);

    obj->type = ehdr->e_type;
    obj->ehdr = mem_dup(ehdr, sizeof(Elf32_Ehdr));
    obj->shdr = mem_dup(shdr, ehdr->e_shnum * sizeof(Elf32_Shdr));

    load_sections(obj, file);

    uint8_t *o = NULL;
    if (obj->type != ET_EXEC)
        o = obj->mem;

    for (int i = 0; i < ehdr->e_shnum; i++) {
        switch (shdr[i].sh_type) {
            case SHT_SYMTAB:
                obj->sym_tab = mem_dup(file + shdr[i].sh_offset, shdr[i].sh_size);
                obj->sym_str = mem_dup(file + shdr[shdr[i].sh_link].sh_offset, shdr[shdr[i].sh_link].sh_size);
                obj->sym_size = shdr[i].sh_size;
                break;

            case SHT_DYNSYM:
                obj->dym_tab = (Elf32_Sym *)(o + shdr[i].sh_addr);
                obj->dym_str = (char *) o + shdr[shdr[i].sh_link].sh_addr;
                obj->dym_size = shdr[i].sh_size;
                break;

            case SHT_DYNAMIC:
                obj->dynamic = (Elf32_Dyn *)(o + shdr[i].sh_addr);
                break;

            default:
                break;
        }
    }

    mmq_free(file);

    if (obj->dym_tab == NULL) {
        if (obj->type == ET_DYN) {
            if (isfatal)
                raise_error("%s: no dynamic section", path);
            free_elf(obj);
            return NULL;
        }
        goto endopen;
    }

    obj->hash_table = hash_create(obj);

    if (obj->type == ET_DYN)
        loaded_lib_add(obj);

    if (rlvl < 0)
        return obj;

    if (obj->dynamic == NULL)
        goto endopen;

    int deps_count = 0;
    for (int i = 0; obj->dynamic[i].d_tag != 0; i++) {
        if (obj->dynamic[i].d_tag == DT_NEEDED)
            deps_count++;
    }

    if (deps_count == 0)
        goto endopen;

    obj->deps = mmq_calloc(deps_count + 1, sizeof(elfobj_t *));
    deps_count = 0;

    for (int i = 0; obj->dynamic[i].d_tag != 0; i++) {
        if (obj->dynamic[i].d_tag != DT_NEEDED)
            continue;

        char *name = (char *) obj->dym_str + obj->dynamic[i].d_un.d_val;

        if ((obj->deps[deps_count] = open_elf_r(name, ET_DYN, isfatal,
                visibility == VISIBILITY_GLOBAL ? VISIBILITY_LOCAL : visibility,
                rlvl + 1))
        ) {
            deps_count++;
            continue;
        }

        free_elf(obj);
        return NULL;
    }

    if (obj->type == ET_DYN)
        loaded_lib_atback(obj);

    endopen:

    dynamic_linker(obj);
    elfobj_iof_r(obj, 0, rlvl + 1);

    return obj;
}

/********************************
 *                             *
 *    ELF closing functions    *
 *                             *
********************************/

int close_elf_r(elfobj_t *obj, int r_lvl) {
    if (--obj->ref_count > 0) {
        debug_printf_tab(2, r_lvl, "dref %s (%d)", obj->name, obj->ref_count);
        return 0;
    }

    debug_printf_tab(2, r_lvl, "free %s", obj->name);

    elfobj_iof_r(obj, 1, r_lvl + 1);

    if (obj->deps) {
        int count;
        for (count = 0; obj->deps[count]; count++);
        for (int i = count - 1; i >= 0; i--)
            close_elf_r(obj->deps[i], r_lvl + 1);
    }

    // remove from loaded libs
    if (obj->type == ET_DYN) {
        for (int i = 0; i < g_lib_count; i++) {
            if (g_loaded_libs[i] != obj)
                continue;
            g_lib_count--;
            for (int j = i; j < g_lib_count; j++)
                g_loaded_libs[j] = g_loaded_libs[j + 1];
            break;
        }
    }

    free_elf(obj);
    return 0;
}

/********************************
 *                             *
 *  dlfcn interface functions  *
 *                             *
********************************/

#define DLG_NOERR    0
#define DLG_NOFILE   1
#define DLG_NOSYM    2
#define DLG_INVARG   3
#define DLG_INTERNAL 4
#define DLG_NOTIMPL  5

void *dlg_open(const char *filename, int flag) {
    if (filename == NULL) {
        g_dlfcn_error = DLG_INVARG;
        return NULL;
    }

    if (flag & RTLD_TRACE || flag & RTLD_NOLOAD || flag & RTLD_NODELETE) {
        g_dlfcn_error = DLG_NOTIMPL;
        return NULL;
    }

    elfobj_t *obj = open_elf(filename, ET_DYN, 0, flag & RTLD_GLOBAL ? VISIBILITY_GLOBAL : VISIBILITY_LOCAL);
    g_dlfcn_error = obj ? DLG_NOERR : DLG_NOFILE;

    return obj;
}

void *dlg_sym(void *handle, const char *symbol) {
    elfobj_t *obj = handle;
    uint32_t val;

    if (obj == NULL || symbol == NULL) {
        g_dlfcn_error = DLG_INVARG;
        return NULL;
    }

    if (obj == RTLD_DEFAULT) {
        val = search_sym_value(symbol, NULL,
                1,  // allow global libraries
                1   // check local symbols
        );
        g_dlfcn_error = val ? DLG_NOERR : DLG_NOSYM;
        return (void *) val;
    }

    if (obj == RTLD_NEXT) {
        g_dlfcn_error = DLG_NOTIMPL;
        return NULL;
    }

    val = search_sym_value(symbol, obj,
            0,  // don't allow global libraries
            1   // check local symbols
    );
    g_dlfcn_error = val ? DLG_NOERR : DLG_NOSYM;

    return (void *) val;
}

int dlg_close(void *handle) {
    if (handle == NULL)
        g_dlfcn_error = DLG_INVARG;
    else if (close_elf(handle))
        g_dlfcn_error = DLG_INTERNAL;
    else
        g_dlfcn_error = DLG_NOERR;

    return g_dlfcn_error == DLG_NOERR ? 0 : -1;
}

char *dlg_error(void) {
    char *error;
    switch (g_dlfcn_error) {
        case DLG_NOERR:
            return NULL;
        case DLG_NOFILE:
            error = "deluge: file not found";
            break;
        case DLG_NOSYM:
            error = "deluge: symbol not found";
            break;
        case DLG_INVARG:
            error = "deluge: invalid argument";
            break;
        case DLG_INTERNAL:
            error = "deluge: internal error";
            break;
        case DLG_NOTIMPL:
            error = "deluge: not implemented flag";
            break;
        default:
            error = "deluge: unknown error";
            break;
    }
    g_dlfcn_error = DLG_NOERR;
    return error;
}

char *dlg_fn_name(void *ptr, char **libname) {
    uint32_t addr = (uint32_t) ptr;
    char *name;

    // look inside the g_prog
    if ((name = get_addr_name(g_prog, addr))) {
        if (libname)
            *libname = g_prog->name;
        return name;
    }

    // look inside the loaded libraries
    for (int i = 0; i < g_lib_count; i++) {
        if ((name = get_addr_name(g_loaded_libs[i], addr))) {
            if (libname)
                *libname = g_loaded_libs[i]->name;
            return name;
        }
    }

    if (libname)
        *libname = NULL;
    return NULL;
}

/********************************
 *                             *
 *  LibC interface functions   *
 *                             *
********************************/

void libc_enable_leaks(void) {
    void (*buddy_enable_leaks)(void) = (void *) search_sym_value("__buddy_enable_leaks", NULL, 1, 0);

    if (buddy_enable_leaks) {
        buddy_enable_leaks();
    } else {
        raise_error("failed to enable leaks");
    }
}

/********************************
 *                             *
 *    Command line Interface   *
 *                             *
********************************/

void show_help(int err) {
    if (err) {
        mmq_printf(2, "Try 'deluge -h' for more information.\n");
    } else {
        mmq_printf(1,
            "Usage: deluge [options] <file> [args]\n"
            "Options:\n"
            "  -a   import an additional library\n"
            "  -d   show additional debug information\n"
            "  -e   don't use filename as argument\n"
            "  -h   show this help message and exit\n"
            "  -l   show information about opened libs\n"
            "  -L   add path for libraries search\n"
            "  -m   show memory leaks at exit\n"
            "  -p   load shared object as patch file\n"
            "  -v   dump deluge version and exit\n"
        );
    }

    mmq_exit(err);
}

void deluge_parse(int argc, char **argv) {
    mmq_memset(g_args, 0, sizeof(deluge_args_t));

    g_args->debug = ALWAYS_DEBUG;
    int move_arg = 0;

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-') {
            g_args->name = argv[i];
            g_args->arg_offset = i + move_arg;
            break;
        }
        switch (argv[i][1]) {
            case 'd':
                g_args->debug = 2;
                break;
            case 'e':
                move_arg = 1;
                break;
            case 'h':
                show_help(0);
                break; // unreachable
            case 'l':
                if (g_args->debug == 0)
                    g_args->debug = 1;
                break;
            case 'a':
                if (i + 1 >= argc) {
                    mmq_printf(2, "deluge: missing argument for -l\n");
                    show_help(1);
                }
                if (g_args->extra_lib) {
                    mmq_printf(2, "deluge: extra library already set\n");
                    show_help(1);
                }
                g_args->extra_lib = argv[++i];
                break;
            case 'L':
                if (i + 1 >= argc) {
                    mmq_printf(2, "deluge: missing argument for -L\n");
                    show_help(1);
                }
                if (g_args->extra_path) {
                    mmq_printf(2, "deluge: lib path already set\n");
                    show_help(1);
                }
                g_args->extra_path = argv[++i];
                break;
            case 'm':
                g_args->show_leaks = 1;
                break;
            case 'p':
                if (i + 1 >= argc) {
                    mmq_printf(2, "deluge: missing argument for -p\n");
                    show_help(1);
                }
                if (g_args->patch_file) {
                    mmq_printf(2, "deluge: patch file already set\n");
                    show_help(1);
                }
                g_args->patch_file = argv[++i];
                break;
            case 'v':
                mmq_printf(1, "deluge %s\n", DELUGE_VERSION);
                mmq_exit(0);
                break; // unreachable
            case '\0':
            case '-':
                mmq_printf(2, "deluge: unknown option '%s'\n", argv[i]);
                show_help(1);
                break; // unreachable
            default:
                mmq_printf(2, "deluge: invalid option -- '%c'\n", argv[i][1]);
                show_help(1);
                break; // unreachable
        }
    }

    if (g_args->name == NULL) {
        mmq_printf(2, "deluge: missing file name\n");
        show_help(1);
    }
}

int main(int argc, char **argv, char **envp) {
    g_dlfcn_error = g_lib_count = 0;
    g_loaded_libs = NULL;
    g_prog = NULL;

    elfobj_t *patch_file, *extra_lib;

    deluge_args_t args;
    g_args = &args;
    g_envp = envp;

    deluge_parse(argc, argv);

    int ret, start, pid, parent_pid = syscall_process_pid();

    if (g_args->debug) {
        start = syscall_ms_get();
    } else {
        start = 0;
    }

    if (g_args->patch_file && !(patch_file = open_elf_r(g_args->patch_file, ET_DYN, 0, VISIBILITY_INIT, -1))) {
        raise_error("%s: failed to open patch file", g_args->patch_file);
        return 1;
    }

    if ((g_prog = open_elf(g_args->name, 0, 1, VISIBILITY_INIT)) == NULL) {
        raise_error("%s: failed to open file", g_args->name);
        return 1;
    }

    if (g_args->extra_lib && !(extra_lib = open_elf(g_args->extra_lib, ET_DYN, 0, VISIBILITY_INIT))) {
        raise_error("%s: failed to open extra library", g_args->extra_lib);
        return 1;
    }

    if (g_args->patch_file) {
        dynamic_linker(patch_file);
        elfobj_iof(patch_file, 0);
    }

    if (g_args->show_leaks) {
        libc_enable_leaks();
    }

    if (g_prog->type == ET_EXEC) {
        debug_printf(1, "link time: %d ms", syscall_ms_get() - start);

        int (*main)(int, char **, char **) = (void *) g_prog->ehdr->e_entry;

        if (argc > g_args->arg_offset) {
            syscall_process_info(syscall_process_pid(), PROC_INFO_SET_NAME, argv[g_args->arg_offset]);
        }

        if (g_args->debug) {
            start = syscall_ms_get();
        }

        ret = main(argc - g_args->arg_offset, argv + g_args->arg_offset, envp);

        pid = syscall_process_pid();

        if (pid != parent_pid) {
            debug_printf(1, "PID %d (child of %d) process_exit with code %d after %d ms",
                    pid, parent_pid, ret, syscall_ms_get() - start);
            syscall_process_kill(pid, ret);
        }

        debug_printf(1, "PID %d process_exit with code %d in %d ms", pid, ret, syscall_ms_get() - start);
    } else {
        debug_printf(1, "no entry point in shared object");
        ret = 0;
    }

    close_elf(g_prog);

    if (g_args->extra_lib)
        close_elf(extra_lib);
    if (g_args->patch_file)
        close_elf(patch_file);

    for (int i = 0; i < g_lib_count; i++) {
        debug_printf(1, "unclosed library '%s'", g_loaded_libs[i]->name);
        free_elf(g_loaded_libs[i]);
    }

    mmq_free(g_loaded_libs);

    if (g_args->show_leaks) {
        int leaks;
        if ((leaks = syscall_mem_info(7, parent_pid)) > 0) {
            mmq_printf(2, "\n  Kernel memory leak of %d alloc%s (pid %d, %d bytes)\n",
                    leaks, leaks == 1 ? "" : "s", parent_pid,
                    syscall_mem_info(8, parent_pid)
            );
        }
    }

    return ret;
}
