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

#define _SYSCALL_CREATE_FUNCS

#include <profan/syscall.h>
#include <profan/filesys.h>
#include <profan/libmmq.h>
#include <profan/dlgext.h>

#include <dlfcn.h>

#define DELUGE_VERSION  "4.1"
#define ALWAYS_DEBUG    0
#define USE_CACHED_LIBC 1

/****************************
 *                         *
 *    Types and globals    *
 *                         *
****************************/

char *profan_fn_name(void *ptr, char **libname);

#define raise_error(fmt, ...) do {  \
            fd_printf(2, "DELUGE FATAL: "fmt"\n", ##__VA_ARGS__); \
            process_exit(1);  \
        } while (0)

#define debug_printf(lvl, fmt, ...) if (g_args->debug >= lvl) {  \
            fd_putstr(2, "\e[37m[DELUGE] ");  \
            fd_printf(2, fmt, __VA_ARGS__);  \
            fd_putstr(2, "\e[0m\n");  \
        }

// internal variables
elfobj_t **g_loaded_libs;
elfobj_t *g_prog;

int    g_already_fini;
int    g_lib_count;
char **g_envp;

// command line options
char *g_extralib_path;
int   g_dlfcn_error;

// extra symbols structure
typedef struct {
    const char *name;
    uint32_t hash;
    void *data;
} dlg_extra_t;

// extra symbols table
dlg_extra_t g_extra_syms[] = {
    { "profan_fn_name", 0x62289205, profan_fn_name },
    { "dlclose"       , 0xDE67CAC5, dlclose        },
    { "dlopen"        , 0xCEF94D0E, dlopen         },
    { "dlsym"         , 0x0677DB8D, dlsym          },
    { "dlerror"       , 0xDE8AD652, dlerror        },
    { NULL, 0, NULL }
};

// deluge arguments structure
typedef struct {
    char   *name;
    char   *extra_lib;
    uint8_t show_leaks;
    int     arg_offset;
    int     debug;
} deluge_args_t;

deluge_args_t *g_args;

/********************************
 *                             *
 *      Utility functions      *
 *                             *
********************************/

void loaded_lib_add(elfobj_t *lib) {
    g_loaded_libs = krealloc(g_loaded_libs, ++g_lib_count * sizeof(elfobj_t *));
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

char *ft_getenv(const char *name) {
    uint32_t len = str_len(name);
    for (int i = 0; g_envp[i]; i++) {
        if (str_ncmp(g_envp[i], name, len) == 0 && g_envp[i][len] == '=')
            return g_envp[i] + len + 1;
    }
    return NULL;
}

char *profan_join_path(const char *dir, const char *file) {
    int len1 = str_len(dir);
    char *path = kmalloc(len1 + str_len(file) + 2);
    str_cpy(path, dir);
    path[len1] = '/';
    str_cpy(path + len1 + 1, file);
    return path;
}

/********************************
 *                             *
 *    File search functions    *
 *                             *
********************************/

uint32_t search_inpath(const char *src_path, const char *filename, char **fullpath) {
    char *path = str_dup(src_path);

    char *fullname = kmalloc(str_len(filename) + 5); // 5 => .elf + null
    str_cpy(fullname, filename);
    str_cat(fullname, ".elf");

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
                    *fullpath = profan_join_path(path + start, fullname);
                kfree(fullname);
                kfree(path);
                return sid;
            }
        }
        if (src_path[i] == '\0')
            break;
        start = i + 1;
    }

    kfree(fullname);
    kfree(path);
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
            *path = str_dup(name);
        return sid;
    }

    if (name[0] == '.' && name[1] == '/') {
        char *cwd = ft_getenv("PWD");
        if (!cwd)
            return SID_NULL;
        full_path = profan_join_path(cwd, name + 2);
        fu_simplify_path(full_path);
        sid = fu_path_to_sid(SID_ROOT, full_path);
        if (!IS_SID_NULL(sid) && path)
            *path = full_path;
        else
            kfree(full_path);
        return sid;
    }

    if (!islib) {
        char *env_path = ft_getenv("PATH");
        if (!env_path)
            return SID_NULL;
        return search_inpath(env_path, name, path);
    }

    if (g_extralib_path) {
        full_path = profan_join_path(g_extralib_path, name);
        sid = fu_path_to_sid(SID_ROOT, full_path);
        if (!IS_SID_NULL(sid)) {
            if (path)
                *path = full_path;
            return sid;
        }
        kfree(full_path);
    }

    full_path = profan_join_path("/lib", name);
    sid = fu_path_to_sid(SID_ROOT, full_path);

    if (!IS_SID_NULL(sid)) {
        if (path)
            *path = full_path;
        return sid;
    }

    kfree(full_path);
    return SID_NULL;
}

/********************************
 *                             *
 *    Hash table functions     *
 *                             *
********************************/

static inline uint32_t hash(const char *str) {
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

        table[table_index].next = entry->next;
        entry->next = &table[table_index];

        table_index++;
    }
    kfree(later);

    return table;
}

Elf32_Sym *hash_get(elfobj_t *obj, uint32_t full_h, const char *key) {
    dlg_hash_t *entry = obj->hash_table + full_h % (obj->dym_size / sizeof(Elf32_Sym));;

    while (entry) {
        if (entry->hash == full_h && str_cmp(entry->key, key) == 0)
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
        mem_cmp(ehdr->e_ident, (void *) ELFMAG, SELFMAG) != 0 ||
        (required_type && ehdr->e_type != required_type) ||
        ehdr->e_machine != EM_386
    );
}

/********************************
 *                             *
 *    ELF loading functions    *
 *                             *
********************************/

void *get_base_addr(uint8_t *data, uint16_t type) {
    if (type != ET_EXEC)
        return 0;

    // find the lowest address of a PT_LOAD segment
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)data;
    Elf32_Phdr *phdr = (Elf32_Phdr *)(data + ehdr->e_phoff);

    uint32_t base_addr = 0xFFFFFFFF;

    for (int i = 0; i < ehdr->e_phnum; i++) {
        if (phdr[i].p_type == 1 && phdr[i].p_vaddr < base_addr)
            base_addr = phdr[i].p_vaddr;
    }

    return (void *) base_addr;
}

int load_sections(elfobj_t *obj) {
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)obj->file;
    Elf32_Shdr *shdr = (Elf32_Shdr *)(obj->file + ehdr->e_shoff);

    void *base_addr = get_base_addr(obj->file, obj->type);
    void *offset = NULL;

    uint32_t required_size = 0;

    for (int i = 0; i < ehdr->e_shnum; i++) {
        if (shdr[i].sh_addr + shdr[i].sh_size > required_size)
            required_size = shdr[i].sh_addr + shdr[i].sh_size;
    }

    if (obj->type == ET_EXEC)
        required_size -= (uint32_t) base_addr;
    required_size = (required_size + 0xFFF) & ~0xFFF;

    if (obj->type == ET_EXEC) {
        obj->mem = (void *) base_addr;
        syscall_scuba_generate(base_addr, required_size / 0x1000);
    } else {
        obj->mem = (void *) syscall_mem_alloc(required_size, 0x1000, 1);
        offset = obj->mem;
    }

    for (int i = 0; i < ehdr->e_shnum; i++) {
        if (!shdr[i].sh_addr)
            continue;
        switch (shdr[i].sh_type) {
            case SHT_NOBITS:
                mem_set(offset + shdr[i].sh_addr, 0, shdr[i].sh_size);
                break;
            default:
                mem_cpy(offset + shdr[i].sh_addr, obj->file + shdr[i].sh_offset, shdr[i].sh_size);
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

/* == 32-bit x86 relocations types ==
 *
 * R_386_NONE
 * R_386_32         S + A
 * R_386_PC32       S + A - P
 * R_386_GOT32      G + A
 * R_386_PLT32      L + A - P
 * R_386_COPY
 * R_386_GLOB_DAT   S
 * R_386_JMP_SLOT   S
 * R_386_RELATIVE   B + A
 * R_386_GOTOFF     S + A - GOT
 * R_386_GOTPC      GOT + A - P
 * R_386_32PLT      L + A

 * S:   value of the symbol
 * A:   addend
 * P:   place of the storage unit being relocated
 * B:   base address of the shared object
 * GOT: address of the global offset table
 * L:   address of the procedure linkage table
*/

int does_type_required_sym(uint8_t type) {
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

char *get_addr_name(elfobj_t *obj, uint32_t addr) {
    char *name = NULL;

    if (!obj->sym_tab)
        return NULL;

    for (uint32_t i = 0; i < obj->sym_size / sizeof(Elf32_Sym); i++) {
        uint32_t val = (uint32_t) obj->mem + obj->sym_tab[i].st_value;
        if (addr < val || addr >= val + obj->sym_tab[i].st_size)
            continue;
        name = obj->sym_str + obj->sym_tab[i].st_name;
    }

    return name;
}

uint32_t get_sym_extra(const char *name, uint32_t full_h) {
    for (int i = 0; g_extra_syms[i].name; i++) {
        if (g_extra_syms[i].hash == full_h && str_cmp(g_extra_syms[i].name, name) == 0)
            return (uint32_t) g_extra_syms[i].data;
    }
    return 0;
}

uint32_t get_sym_value(const char *name, Elf32_Sym **sym_ptr) {
    Elf32_Sym *sym;

    uint32_t val, full_h = hash(name);

    val = get_sym_extra(name, full_h);
    if (val) return val;

    for (int k = 0; k < g_lib_count; k++) {
        sym = hash_get(g_loaded_libs[k], full_h, name);
        if (!sym || sym->st_shndx == STB_LOCAL)
            continue;
        if (sym_ptr)
            *sym_ptr = sym;
        return (uint32_t) sym->st_value + (uint32_t) g_loaded_libs[k]->mem;
    }
    return 0;
}

int dynamic_linker(elfobj_t *obj) {
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)obj->file;
    Elf32_Shdr *shdr = (Elf32_Shdr *)(obj->file + ehdr->e_shoff);

    debug_printf(2, "link %s", obj->name);

    if (obj->dym_tab == NULL) {
        return 0;
    }

    uint32_t val, *ptr;
    uint8_t type;
    char *name;

    for (uint32_t i = 0; i < ehdr->e_shnum; i++) {
        if (shdr[i].sh_type == SHT_RELA)
            raise_error("SHT_RELA is not supported but found in '%s'", obj->name);

        if (shdr[i].sh_type != SHT_REL)
            continue;

        Elf32_Rel *rel = (Elf32_Rel *)(obj->file + shdr[i].sh_offset);

        for (uint32_t j = 0; j < shdr[i].sh_size / sizeof(Elf32_Rel); j++) {
            name = (char *) obj->dym_str + (obj->dym_tab + ELF32_R_SYM(rel[j].r_info))->st_name;


            type = ELF32_R_TYPE(rel[j].r_info);
            if (does_type_required_sym(type)) {
                val = get_sym_value(name, NULL);
                if (val == 0) {
                    if (obj->type == ET_DYN)
                        raise_error("%s: symbol '%s' not found", obj->name, name);
                    Elf32_Sym *sym = hash_get(obj, hash(name), name); // ET_EXEC
                    if (!sym || sym->st_shndx == STB_LOCAL)
                        raise_error("%s: symbol '%s' not found", obj->name, name);
                    val = sym->st_value;
                }

            } else val = 0; // for gcc -O3

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
                    *ptr = val;
                    break;
                case R_386_RELATIVE:    // word32  B + A
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
 *    ELF opening functions    *
 *                             *
********************************/

void *open_elf(const char *filename, uint16_t required_type, int isfatal) {
    char *path = NULL;
    uint32_t sid;

    #if USE_CACHED_LIBC
    static elfobj_t *libc = NULL;
    #endif

    sid = search_elf_sid(filename, required_type == ET_DYN, &path);

    if (IS_SID_NULL(sid)) {
        if (isfatal)
            raise_error("%s: file not found", filename);
        return NULL;
    }

    if (required_type == ET_DYN) {
        for (int i = 0; i < g_lib_count; i++) {
            if (str_cmp(g_loaded_libs[i]->name, path))
                continue;
            debug_printf(2, "find %s", path);
            kfree(path);
            return g_loaded_libs[i];
        }

        #if USE_CACHED_LIBC
        if (str_cmp(path, "/lib/libc.so") == 0) {
            debug_printf(1, "open %s (cached)", path);
            if (libc == NULL)
                libc = dlgext_libc();
            loaded_lib_add(libc);
            kfree(path);
            return libc;
        }
        #endif
    }

    debug_printf(1, "open %s", path);

    elfobj_t *obj = kcalloc(1, sizeof(elfobj_t));

    obj->size = fu_file_get_size(sid);
    obj->file = kmalloc(obj->size);
    obj->ref_count = 1;
    obj->need_free = 1;
    obj->name = path;

    fu_file_read(sid, obj->file, 0, obj->size);

    if (obj->size < sizeof(Elf32_Ehdr) || !is_valid_elf(obj->file, required_type)) {
        if (isfatal)
            raise_error("%s: invalid ELF file", path);
        kfree(obj->file);
        kfree(obj->name);
        kfree(obj);
        return NULL;
    }

    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)obj->file;
    Elf32_Shdr *shdr = (Elf32_Shdr *)(obj->file + ehdr->e_shoff);
    obj->type = ehdr->e_type;

    for (int i = 0; i < ehdr->e_shnum; i++) {
        switch (shdr[i].sh_type) {
            case SHT_SYMTAB:
                obj->sym_tab = (Elf32_Sym *)(obj->file + shdr[i].sh_offset);
                obj->sym_str = (char *) obj->file + shdr[shdr[i].sh_link].sh_offset;
                obj->sym_size = shdr[i].sh_size;
                break;

            case SHT_DYNAMIC:
                obj->dynamic = (Elf32_Dyn *)(obj->file + shdr[i].sh_offset);
                break;

            case SHT_DYNSYM:
                obj->dym_tab = (Elf32_Sym *)(obj->file + shdr[i].sh_offset);
                obj->dym_str = (char *) obj->file + shdr[shdr[i].sh_link].sh_offset;
                obj->dym_size = shdr[i].sh_size;
                break;

            default:
                break;
        }
    }

    if (obj->dym_tab == NULL) {
        if (obj->type == ET_DYN) {
            if (isfatal)
                raise_error("%s: no dynamic section", path);
            kfree(obj->file);
            kfree(obj->name);
            kfree(obj);
            return NULL;
        }
        return obj;
    }

    obj->hash_table = hash_create(obj);

    if (obj->type == ET_DYN)
        loaded_lib_add(obj);

    if (obj->dynamic == NULL)
        return obj;

    for (int i = 0; obj->dynamic[i].d_tag != 0; i++) {
        if (obj->dynamic[i].d_tag != DT_NEEDED)
            continue;

        char *name = (char *) obj->dym_str + obj->dynamic[i].d_un.d_val;
        if (open_elf(name, ET_DYN, 0) == NULL)
            raise_error("%s: failed to open needed library '%s'", path, name);
    }

    if (obj->type == ET_DYN)
        loaded_lib_atback(obj);

    return obj;
}

/********************************
 *                             *
 *   ELF init/fini functions   *
 *                             *
********************************/

void elfobj_iof(elfobj_t *obj, int fini) {
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
                debug_printf(2, "call %s %s()", obj->name, name);
            } else {
                debug_printf(2, "call %s [%p]", obj->name, func_array[i]);
            }
        }
        func_array[i]();
    }
}

/********************************
 *                             *
 *  dlfcn interface functions  *
 *                             *
********************************/

void *dlopen(const char *filename, int flag) {
    if (filename == NULL) {
        g_dlfcn_error = 1;
        return NULL;
    }

    g_dlfcn_error = 0;

    elfobj_t *dl = open_elf(filename, ET_DYN, flag == RTLD_FATAL);

    if (dl == NULL) {
        g_dlfcn_error = 1;
        return NULL;
    }

    if (dl->mem != NULL) {
        dl->ref_count++;
        return dl;
    }

    load_sections(dl);
    dynamic_linker(dl);
    elfobj_iof(dl, 0);
    return dl;
}

void *dlsym(void *handle, const char *symbol) {
    elfobj_t *dl = handle;
    uint32_t val, full_h;

    g_dlfcn_error = 0;

    if (dl == NULL) {
        val = get_sym_value(symbol, NULL);
        if (val)
            return (void *) val;
        g_dlfcn_error = 2;
        return NULL;
    }

    Elf32_Ehdr *ehdr = (Elf32_Ehdr *) dl->file;
    Elf32_Sym *ret;

    full_h = hash(symbol);

    if (ehdr->e_type == ET_EXEC) {
        ret = hash_get(dl, full_h, symbol);

        if (ret)
            return (void *) ret->st_value;
        g_dlfcn_error = 2;
        return NULL;
    }

    if ((val = get_sym_extra(symbol, full_h))) {
        return (void *) val;
    }

    ret = hash_get(dl, full_h, symbol);

    if (ret)
        return (void *) dl->mem + ret->st_value;
    g_dlfcn_error = 2;
    return NULL;
}

int dlclose(void *handle) {
    if (handle == NULL)
        return 0;

    elfobj_t *dl = handle;

    if (--dl->ref_count > 0) {
        debug_printf(2, "dref %s (%d)", dl->name, dl->ref_count);
        return 0;
    }

    debug_printf(2, "free %s", dl->name);

    // remove from loaded libs
    for (int i = 0; i < g_lib_count; i++) {
        if (g_loaded_libs[i] != dl)
            continue;
        g_lib_count--;
        for (int j = i; j < g_lib_count; j++) {
            g_loaded_libs[j] = g_loaded_libs[j + 1];
        }
        break;
    }

    if (!g_already_fini) {
        elfobj_iof(dl, 1);
    }

    if (dl->need_free) {
        kfree(dl->hash_table);
        kfree(dl->file);
        kfree(dl->name);
    }

    kfree(dl->mem);
    kfree(dl);
    return 0;
}

char *dlerror(void) {
    char *error;
    switch (g_dlfcn_error) {
        case 0:
            return NULL;
        case 1:
            error = "deluge dlfcn: failed to open file";
            break;
        case 2:
            error = "deluge dlfcn: symbol not found";
            break;
        default:
            error = "deluge dlfcn: unknown error";
            break;
    }
    g_dlfcn_error = 0;
    return error;
}

char *profan_fn_name(void *ptr, char **libname) {
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
    void (*buddy_enable_leaks)(void) = (void *) get_sym_value("__buddy_enable_leaks", NULL);

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

void show_help(int full) {
    if (!full) {
        fd_printf(2, "Try 'deluge -h' for more information.\n");
        process_exit(1);
    }
    fd_printf(1,
        "Usage: deluge [options] <file> [args]\n"
        "Options:\n"
        "  -a  add an extra library\n"
        "  -d  show additional debug info\n"
        "  -e  don't use filename as argument\n"
        "  -h  show this help message\n"
        "  -l  list loaded files\n"
        "  -m  show userspace memory leaks\n"
        "  -p  add path to extra libraries\n"
        "  -v  show version\n"
    );
}

void deluge_parse(int argc, char **argv) {
    g_args->extra_lib = NULL;
    g_args->name = NULL;

    g_args->show_leaks = 0;

    g_args->debug = ALWAYS_DEBUG;
    g_extralib_path = NULL;

    int move_arg = 0;

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-') {
            g_args->name = argv[i];
            g_args->arg_offset = i + move_arg;
            break;
        }
        switch (argv[i][1]) {
            case 'a':
                if (i + 1 >= argc) {
                    fd_printf(2, "deluge: missing argument for -a\n");
                    show_help(0);
                }
                if (g_args->extra_lib) {
                    fd_printf(2, "deluge: extra library already set\n");
                    show_help(0);
                }
                g_args->extra_lib = argv[++i];
                break;
            case 'l':
                if (g_args->debug == 0)
                    g_args->debug = 1;
                break;
            case 'd':
                g_args->debug = 2;
                break;
            case 'e':
                move_arg = 1;
                break;
            case 'h':
                show_help(1);
                process_exit(0);
                break; // unreachable
            case 'm':
                g_args->show_leaks = 1;
                break;
            case 'p':
                if (i + 1 >= argc) {
                    fd_printf(2, "deluge: missing argument for -p\n");
                    show_help(0);
                }
                if (g_extralib_path) {
                    fd_printf(2, "deluge: extra library path already set\n");
                    show_help(0);
                }
                g_extralib_path = argv[++i];
                break;
            case 'v':
                fd_printf(1, "deluge %s\n", DELUGE_VERSION);
                process_exit(0);
                break; // unreachable
            case '\0':
            case '-':
                fd_printf(2, "deluge: unknown option '%s'\n", argv[i]);
                show_help(0);
                break; // unreachable
            default:
                fd_printf(2, "deluge: invalid option -- '%c'\n", argv[i][1]);
                show_help(0);
                break; // unreachable
        }
    }

    if (g_args->name == NULL) {
        fd_printf(2, "deluge: missing file name\n");
        show_help(0);
    }
}

int main(int argc, char **argv, char **envp) {
    g_loaded_libs = NULL;
    g_already_fini = 0;
    g_dlfcn_error = 0;
    g_lib_count = 0;
    g_envp = envp;

    deluge_args_t args;
    g_args = &args;

    deluge_parse(argc, argv);

    int ret, start, pid, parent_pid = syscall_process_pid();

    if (g_args->debug) {
        start = syscall_timer_get_ms();
    } else {
        start = 0;
    }

    if (g_args->extra_lib) {
        if (open_elf(g_args->extra_lib, ET_DYN, 0) == NULL) {
            raise_error("%s: failed to open extra library", g_args->extra_lib);
            return 1;
        }
    }

    g_prog = open_elf(g_args->name, 0, 1);

    if (g_prog == NULL) {
        raise_error("%s: failed to open file", g_args->name);
        return 1;
    }

    for (int i = 0; i < g_lib_count; i++) {
        load_sections(g_loaded_libs[i]);
    }

    for (int i = 0; i < g_lib_count; i++) {
        dynamic_linker(g_loaded_libs[i]);
    }

    for (int i = 0; i < g_lib_count; i++) {
        elfobj_iof(g_loaded_libs[i], 0);
    }

    if (g_args->show_leaks) {
        libc_enable_leaks();
    }

    if (g_prog->type == ET_EXEC) {
        load_sections(g_prog);
        dynamic_linker(g_prog);

        debug_printf(1, "link time: %d ms", syscall_timer_get_ms() - start);

        int (*main)() = (int (*)(int, char **, char **)) ((Elf32_Ehdr *) g_prog->file)->e_entry;

        kfree(g_prog->hash_table);

        g_dlfcn_error = 0;

        if (argc > g_args->arg_offset) {
            syscall_process_info(syscall_process_pid(), PROC_INFO_SET_NAME, argv[g_args->arg_offset]);
        }

        elfobj_iof(g_prog, 0);

        if (g_args->debug) {
            start = syscall_timer_get_ms();
        }

        ret = main(argc - g_args->arg_offset, argv + g_args->arg_offset, envp);

        pid = syscall_process_pid();

        if (pid != parent_pid) {
            debug_printf(1, "PID %d (child of %d) process_exit with code %d after %d ms",
                    pid, parent_pid, ret, syscall_timer_get_ms() - start);
            syscall_process_kill(pid, ret);
        }

        debug_printf(1, "PID %d process_exit with code %d in %d ms", pid, ret, syscall_timer_get_ms() - start);

        elfobj_iof(g_prog, 1);
    } else {
        ret = 0;
    }

    for (int i = 0; i < g_lib_count; i++) {
        elfobj_iof(g_loaded_libs[i], 1);
    }

    g_already_fini = 1;

    while (g_lib_count) {
        if (g_loaded_libs[0]->ref_count > 1) {
            debug_printf(1, "Unclosed library '%s'", g_loaded_libs[0]->name);
            g_loaded_libs[0]->ref_count = 1;
        }
        dlclose(g_loaded_libs[0]);
    }

    kfree(g_loaded_libs);

    if (g_prog->type == ET_EXEC) {
        kfree(g_prog->file);
        kfree(g_prog->name);
        kfree(g_prog);
    }

    if (g_args->show_leaks) {
        int leaks;
        if ((leaks = syscall_mem_info(7, parent_pid)) > 0) {
            fd_printf(2, "\n  Kernel memory leak of %d alloc%s (pid %d, %d bytes)\n\n",
                    leaks, leaks == 1 ? "" : "s", parent_pid,
                    syscall_mem_info(8, parent_pid)
            );
        }
    }

    return ret;
}
