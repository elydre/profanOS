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

#include <profan/syscall.h>
#include <profan/filesys.h>
#include <profan/libmmq.h>
#include <profan/dlgext.h>

#include <dlfcn.h>

#define DELUGE_VERSION "1.6"
#define ALWAYS_DEBUG 0

/****************************
 *                         *
 *    Types and globals    *
 *                         *
****************************/

#define raise_error(fmt, ...) do {  \
            fd_printf(2, "DELUGE FATAL: "fmt"\n", ##__VA_ARGS__); \
            exit(1);  \
        } while (0)

#define debug_printf(fmt, ...) if (g_print_deps) {  \
            fd_putstr(2, "\e[37m[DELUGE] ");  \
            fd_printf(2, fmt, __VA_ARGS__);  \
            fd_putstr(2, "\e[0m\n");  \
        }

// internal variables
elfobj_t **g_loaded_libs;

int g_lib_count;
int g_cleanup;

// command line options
char *g_extralib_path;
int   g_print_deps;

void add_loaded_lib(elfobj_t *lib) {
    g_loaded_libs = realloc(g_loaded_libs, ++g_lib_count * sizeof(elfobj_t *));
    g_loaded_libs[g_lib_count - 1] = lib;
}

/**************************
 *                       *
 *    Find file funcs    *
 *                       *
**************************/

char *assemble_path(const char *dir, const char *file) {
    int len1 = strlen(dir);
    char *path = malloc(len1 + strlen(file) + 2);
    strcpy(path, dir);
    path[len1] = '/';
    strcpy(path + len1 + 1, file);
    return path;
}

sid_t shearch_elf_sid(const char *lib, uint16_t type, char **path) {
    sid_t sid;

    if (type == ET_EXEC || lib[0] == '/') {
        sid = fu_path_to_sid(ROOT_SID, lib);
        if (!IS_NULL_SID(sid) && path)
            *path = strdup(lib);
        return sid;
    }

    char *full_path = assemble_path("/lib", lib);
    sid = fu_path_to_sid(ROOT_SID, full_path);

    if (IS_NULL_SID(sid)) {
        free(full_path);
        if (!g_extralib_path)
            return NULL_SID;
        full_path = assemble_path(g_extralib_path, lib);
        sid = fu_path_to_sid(ROOT_SID, full_path);
        if (IS_NULL_SID(sid))
            free(full_path);
        else if (path)
            *path = full_path;
    } else if (path) {
        *path = full_path;
    }

    return sid;
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
            table[h].data = (void *) obj->dymsym[i].st_value;
            table[h].key = key;
            table[h].hash = full_h;
        } else {
            later[later_index].data = (void *) obj->dymsym[i].st_value;
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

void *hash_get(elfobj_t *obj, const char *key) {
    uint32_t full_h = hash(key);
    dlg_hash_t *entry = obj->sym_table + full_h % (obj->dynsym_size / sizeof(Elf32_Sym));;

    while (entry) {
        if (entry->hash == full_h && strcmp(entry->key, key) == 0)
            return entry->data;
        entry = entry->next;
    }
    return NULL;
}

/************************
 *                     *
 *    ELF functions    *
 *                     *
************************/

int is_valid_elf(void *data, uint16_t required_type) {
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)data;
    return !(
        memcmp(ehdr->e_ident, (void *) ELFMAG, SELFMAG) != 0 ||
        ehdr->e_type != required_type ||
        ehdr->e_machine != EM_386
    );
}

char **get_required_libs(elfobj_t *obj) {
    if (obj->dynamic == NULL) {
        return NULL;
    }

    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)obj->file;
    Elf32_Shdr *shdr = (Elf32_Shdr *)(obj->file + ehdr->e_shoff);

    char **libs = NULL;
    int lib_count = 0;
    int max_libs = 0;

    if (obj->dynamic == NULL) {
        raise_error("no dynamic section found in '%s'", obj->name);
        return NULL;
    }

    for (int i = 0; obj->dynamic[i].d_tag != 0; i++) {
        if (obj->dynamic[i].d_tag != 1) // DT_NEEDED
            continue;
        if (lib_count >= max_libs) {
            max_libs += 16;
            libs = realloc(libs, max_libs * sizeof(char *));
        }
        libs[lib_count++] = (char *) obj->dynstr + obj->dynamic[i].d_un.d_val;
    }

    if (lib_count == 0)
        return NULL;

    libs[lib_count] = NULL;
    return libs;
}

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

int load_sections(elfobj_t *obj, uint16_t type) {
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)obj->file;
    Elf32_Shdr *shdr = (Elf32_Shdr *)(obj->file + ehdr->e_shoff);

    void *base_addr = get_base_addr(obj->file, type);
    uint32_t required_size = 0;

    for (int i = 0; i < ehdr->e_shnum; i++) {
        if (shdr[i].sh_addr + shdr[i].sh_size > required_size)
            required_size = shdr[i].sh_addr + shdr[i].sh_size;
    }

    if (type == ET_EXEC)
        required_size -= (uint32_t) base_addr;
    required_size = (required_size + 0xFFF) & ~0xFFF;

    if (type == ET_EXEC) {
        obj->mem = (void *) base_addr;
        c_scuba_generate(base_addr, required_size / 0x1000);
    } else {
        obj->mem = (void *) c_mem_alloc(required_size, 0x1000, 1);
    }
    memset(obj->mem, 0, required_size);

    for (int i = 0; i < ehdr->e_shnum; i++) {
        if (shdr[i].sh_type == SHT_PROGBITS && shdr[i].sh_addr) {
            if (type == ET_EXEC)
                memcpy((void *) shdr[i].sh_addr, obj->file + shdr[i].sh_offset, shdr[i].sh_size);
            else
                memcpy(obj->mem + shdr[i].sh_addr, obj->file + shdr[i].sh_offset, shdr[i].sh_size);
        }
    }

    return 0;
}

#define R_386_NONE      0   // None
#define R_386_32        1   // word32  S + A
#define R_386_PC32      2   // word32  S + A - P
#define R_386_GOT32     3   // word32  G + A
#define R_386_PLT32     4   // word32  L + A - P
#define R_386_COPY      5   // None
#define R_386_GLOB_DAT  6   // word32  S
#define R_386_JMP_SLOT  7   // word32  S
#define R_386_RELATIVE  8   // word32  B + A
#define R_386_GOTOFF    9   // word32  S + A - GOT
#define R_386_GOTPC     10  // word32  GOT + A - P
#define R_386_32PLT     11  // word32  L + A

// S: value of the symbol
// A: addend
// P: place of the storage unit being relocated
// B: base address of the shared object
// GOT: address of the global offset table
// L: address of the procedure linkage table

int does_type_required_sym(uint8_t type) {
    switch (type) {
        case R_386_32:
        case R_386_PC32:
        case R_386_GLOB_DAT:
        case R_386_JMP_SLOT:
        case R_386_GOTOFF:
            return 1;
        default:
            return 0;
    }
}

int file_relocate(elfobj_t *dl) {
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)dl->file;
    Elf32_Shdr *shdr = (Elf32_Shdr *)(dl->file + ehdr->e_shoff);

    if (ehdr->e_type == ET_EXEC) {
        return 0;
    }

    uint32_t val;
    uint8_t type;
    char *name;

    for (uint32_t i = 0; i < ehdr->e_shnum; i++) {
        if (shdr[i].sh_type == 4) // SHT_RELA
            raise_error("SHT_RELA is not supported but found in '%s'", dl->name);

        if (shdr[i].sh_type != 9) // SHT_REL
            continue;

        Elf32_Rel *rel = (Elf32_Rel *)(dl->file + shdr[i].sh_offset);
        for (uint32_t j = 0; j < shdr[i].sh_size / sizeof(Elf32_Rel); j++) {
            name = (char *) dl->dynstr + (((Elf32_Sym *) dl->dymsym) + ELF32_R_SYM(rel[j].r_info))->st_name;
            val = 0;
            type = ELF32_R_TYPE(rel[j].r_info);
            if (does_type_required_sym(type)) {
                for (int k = 0; k < g_lib_count && val == 0; k++)
                    val = (uint32_t) dlsym(g_loaded_libs[k], name);
                if (val == 0)
                    val = (uint32_t) dlsym(dl, name);
                if (val == 0)
                    raise_error("'%s' requires symbol '%s'", dl->name, name);
            }
            switch (type) {
                case R_386_32:          // word32  S + A
                    val += *(uint32_t *)(dl->mem + rel[j].r_offset);
                    *(uint32_t *)(dl->mem + rel[j].r_offset) = val;
                    break;
                case R_386_PC32:        // word32  S + A - P
                    val += *(uint32_t *)(dl->mem + rel[j].r_offset);
                    val -= (uint32_t) (dl->mem + rel[j].r_offset);
                    *(uint32_t *)(dl->mem + rel[j].r_offset) = val;
                    break;
                case R_386_RELATIVE:    // word32  B + A
                    val = (uint32_t) dl->mem;
                    val += *(uint32_t *)(dl->mem + rel[j].r_offset);
                    *(uint32_t *)(dl->mem + rel[j].r_offset) = val;
                    break;
                case R_386_JMP_SLOT:    // word32  S
                    *(uint32_t *)(dl->mem + rel[j].r_offset) = val;
                    break;
                case R_386_GLOB_DAT:    // word32  S
                    *(uint32_t *)(dl->mem + rel[j].r_offset) = val;
                    break;
                default:
                    raise_error("relocation type %d in '%s' not supported", type, dl->name);
                    break;
            }
        }
    }
    return 0;
}

void *open_elf(const char *filename, uint16_t required_type, int isfatal) {
    static elfobj_t *libc = NULL;
    char *path = NULL;

    if (strcmp(filename, "libc.so")      == 0 ||
        strcmp(filename, "/lib/libc.so") == 0
    ) {
        if (libc) {
            debug_printf("| E-Rf '%s'", filename);
            libc->ref_count++;
            return libc;
        }

        debug_printf("| E-CP '%s'", filename);
        libc = dlgext_libc();
        add_loaded_lib(libc);
        libc->ref_count = 1;
        return libc;
    }

    sid_t sid = shearch_elf_sid(filename, required_type, &path);
    if (IS_NULL_SID(sid)) {
        if (isfatal)
            raise_error("'%s' not found", filename);
        return NULL;
    }

    for (int i = 0; i < g_lib_count; i++) {
        if (strcmp(g_loaded_libs[i]->name, path))
            continue;
        debug_printf("| Find '%s'", path);
        g_loaded_libs[i]->ref_count++;
        return g_loaded_libs[i];
    }

    debug_printf("| Open '%s'", path);

    elfobj_t *obj = calloc(1, sizeof(elfobj_t));

    obj->size = fu_get_file_size(sid);
    obj->file = malloc(obj->size);
    obj->ref_count = 1;
    obj->need_free = 1;
    obj->name = path;

    fu_file_read(sid, obj->file, 0, obj->size);

    if (obj->size < sizeof(Elf32_Ehdr) || !is_valid_elf(obj->file, required_type)) {
        if (isfatal)
            raise_error("'%s' is not a valid ELF file", path);
        free(obj->file);
        free(obj->name);
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
        if (required_type == ET_DYN) {
            if (isfatal)
                raise_error("no dynamic symbol table found in '%s'", path);
            free(obj->file);
            free(obj->name);
            free(obj);
            return NULL;
        }
        return obj;
    }

    obj->sym_table = hash_create(obj);

    if (required_type == ET_DYN) {
        add_loaded_lib(obj);
        return obj;
    }

    char **new_libs = get_required_libs(obj);

    if (new_libs == NULL) {
        return obj;
    }

    for (int i = 0; new_libs[i] != NULL; i++) {
        int found = 0;
        for (int j = 0; g_loaded_libs && j < g_lib_count; j++) {
            if (strcmp(new_libs[i], g_loaded_libs[j]->name))
                continue;
            found = 1;
            break;
        }
        if (found) continue;

        elfobj_t *lib = open_elf(new_libs[i], ET_DYN, isfatal);
        if (lib == NULL)
            raise_error("dlopen failed for '%s'", new_libs[i]);
    }

    free(new_libs);

    return obj;
}

void init_lib(elfobj_t *lib) {
    // call constructors

    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)lib->file;
    Elf32_Shdr *shdr = (Elf32_Shdr *)(lib->file + ehdr->e_shoff);

    if (lib->dynamic == NULL)
        return;

    void (**init_array)(void) = NULL;
    int size = 0;

    for (int j = 0; lib->dynamic[j].d_tag != 0; j++) {
        if (lib->dynamic[j].d_tag == 25) { // DT_INIT
            init_array = (void (**)(void)) (lib->mem + lib->dynamic[j].d_un.d_ptr);
        }
        if (lib->dynamic[j].d_tag == 27) { // DT_INIT_ARRAYSZ
            size = lib->dynamic[j].d_un.d_val / sizeof(void *);
        }
    }

    if (init_array == NULL)
        return;

    for (int i = 0; i < size; i++) {
        init_array[i]();
    }
}

void fini_lib(elfobj_t *lib) {
    // call destructors

    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)lib->file;
    Elf32_Shdr *shdr = (Elf32_Shdr *)(lib->file + ehdr->e_shoff);

    if (lib->dynamic == NULL)
        return;

    void (**fini_array)(void) = NULL;
    int size = 0;

    for (int j = 0; lib->dynamic[j].d_tag != 0; j++) {
        if (lib->dynamic[j].d_tag == 26) { // DT_FINI
            fini_array = (void (**)(void)) (lib->mem + lib->dynamic[j].d_un.d_ptr);
        }
        if (lib->dynamic[j].d_tag == 28) { // DT_FINI_ARRAYSZ
            size = lib->dynamic[j].d_un.d_val / sizeof(void *);
        }
    }

    if (fini_array == NULL)
        return;

    for (int i = 0; i < size; i++) {
        fini_array[i]();
    }
}

void profan_cleanup(void) {
    g_cleanup = 1;
}

int dlfcn_error = 0;

void *dlopen(const char *filename, int flag) {
    elfobj_t *dl = open_elf(filename, ET_DYN, flag == RTLD_FATAL);
    if (dl == NULL) {
        dlfcn_error = 1;
        return NULL;
    }

    dlfcn_error = 0;
    if (dl->ref_count > 1)
        return dl;

    load_sections(dl, ET_DYN);
    file_relocate(dl);
    init_lib(dl);
    return dl;
}

void *dlsym(void *handle, const char *symbol) {
    if (handle == NULL) {
        for (int i = 0; i < g_lib_count; i++) {
            void *ret = dlsym(g_loaded_libs[i], symbol);
            if (ret) {
                dlfcn_error = 0;
                return ret;
            }
        }
        dlfcn_error = 2;
        return NULL;
    }

    elfobj_t *dl = handle;
    void *ret;

    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)dl->file;

    if (!dl->dymsym) {
        dlfcn_error = 2;
        return NULL;
    }

    dlfcn_error = 0;
    if (ehdr->e_type == ET_EXEC) {
        ret = hash_get(dl, symbol);
        if (!ret)
            dlfcn_error = 2;
        return ret;
    }

    if (*symbol && symbol[0] == 'd' && symbol[1] == 'l') {
        if (strcmp(symbol, "dlclose") == 0) {
            return dlclose;
        } else if (strcmp(symbol, "dlopen") == 0) {
            return dlopen;
        } else if (strcmp(symbol, "dlsym") == 0) {
            return dlsym;
        } else if (strcmp(symbol, "dlerror") == 0) {
            return dlerror;
        }
    }
    if (strcmp(symbol, "profan_cleanup") == 0) {
        return profan_cleanup;
    }

    ret = (void *) hash_get(dl, symbol);
    if (ret) {
        ret = dl->mem + (uint32_t) ret;
    } else {
        dlfcn_error = 2;
    }
    return ret;
}

int dlclose(void *handle) {
    if (handle == NULL)
        return 0;

    elfobj_t *dl = handle;

    if (--dl->ref_count > 0) {
        debug_printf("| Dref '%s' (%d)", dl->name, dl->ref_count);
        return 0;
    }

    debug_printf("| Free '%s'", dl->name);

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

    fini_lib(dl);

    if (dl->need_free) {
        free(dl->sym_table);
        free(dl->file);
        free(dl->name);
    }

    free(dl->mem);
    free(dl);
    return 0;
}

char *dlerror(void) {
    char *error;
    if (!dlfcn_error)
        return NULL;
    if (dlfcn_error == 1)
        error = "deluge dlfcn: failed to open file";
    else
        error = "deluge dlfcn: symbol not found";
    dlfcn_error = 0;
    return error;
}

int dynamic_linker(elfobj_t *exec) {
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)exec->file;
    Elf32_Shdr *shdr = (Elf32_Shdr *)(exec->file + ehdr->e_shoff);

    if (ehdr->e_type != ET_EXEC || !exec->dymsym) {
        return 0;
    }

    uint32_t val;
    uint8_t type;
    char *name;

    for (uint32_t i = 0; i < ehdr->e_shnum; i++) {
        if (shdr[i].sh_type == 4) // SHT_RELA
            raise_error("SHT_RELA is not supported but found in '%s'", exec->name);

        if (shdr[i].sh_type != 9) // SHT_REL
            continue;

        Elf32_Rel *rel = (Elf32_Rel *)(exec->file + shdr[i].sh_offset);
        for (uint32_t j = 0; j < shdr[i].sh_size / sizeof(Elf32_Rel); j++) {
            name = (char *) exec->dynstr + (exec->dymsym + ELF32_R_SYM(rel[j].r_info))->st_name;
            val = 0;
            type = ELF32_R_TYPE(rel[j].r_info);
            if (does_type_required_sym(type)) {
                val = (uint32_t) dlsym(exec, name);
                for (int k = 0; !val && k < g_lib_count; k++)
                    val = (uint32_t) dlsym(g_loaded_libs[k], name);
                if (val == 0)
                    raise_error("'%s' requires symbol '%s'", exec->name, name);
            }
            switch (type) {
                case R_386_32:          // word32  S + A
                    val += *(uint32_t *)(rel[j].r_offset);
                    *(uint32_t *)(rel[j].r_offset) = val;
                    break;
                case R_386_COPY:        // None
                    break;
                case R_386_GLOB_DAT:    // word32  S
                    *(uint32_t *)(rel[j].r_offset) = val;
                    break;
                case R_386_JMP_SLOT:    // word32  S
                    *(uint32_t *)(rel[j].r_offset) = val;
                    break;
                default:
                    raise_error("relocation type %d in '%s' not supported", type, exec->name);
                    break;
            }
        }
    }

    return 0;
}

/*********************************
 *                              *
 *    Command line Interface    *
 *                              *
*********************************/

typedef struct {
    char *name;
    uint8_t bench;
    int arg_offset;
} deluge_args_t;

void show_help(int full) {
    if (!full) {
        fd_printf(2, "Try 'deluge -h' for more information.\n");
        exit(1);
    }
    fd_printf(1,
        "Usage: deluge [options] <file> [args]\n"
        "Options:\n"
        "  -b  bench link time and exit\n"
        "  -d  add debug in stderr\n"
        "  -e  use filename as argument\n"
        "  -h  show this help message\n"
        "  -l  add path to extra libraries\n"
        "  -v  show version\n"
    );
}

deluge_args_t deluge_parse(int argc, char **argv) {
    deluge_args_t args;
    args.name = NULL;
    args.bench = 0;

    g_print_deps = ALWAYS_DEBUG;
    g_extralib_path = NULL;

    int move_arg = 1;

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-') {
            args.name = argv[i];
            args.arg_offset = i + move_arg;
            break;
        }
        switch (argv[i][1]) {
            case 'b':
                args.bench = 1;
                break;
            case 'd':
                g_print_deps = 1;
                break;
            case 'e':
                move_arg = 0;
                break;
            case 'h':
                show_help(1);
                exit(0);
                break; // unreachable
            case 'l':
                if (i + 1 >= argc) {
                    fd_printf(2, "deluge: missing argument for -l\n");
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
                exit(0);
                break; // unreachable
            default:
                fd_printf(2, "deluge: invalid option -- '%c'\n", argv[i][1]);
                show_help(0);
                break; // unreachable
        }
    }

    if (args.name == NULL) {
        fd_printf(2, "No file specified\n");
        show_help(0);
    }

    return args;
}

int main(int argc, char **argv, char **envp) {
    g_loaded_libs = NULL;
    g_lib_count = 0;
    g_cleanup = 0;

    deluge_args_t args = deluge_parse(argc, argv);
    uint32_t start;
    int ret = 0;

    if (args.bench || g_print_deps) {
        start = c_timer_get_ms();
    }

    elfobj_t *prog = open_elf(args.name, ET_EXEC, 1);

    if (prog == NULL) {
        raise_error("failed to open '%s'", args.name);
        return 1;
    }

    for (int i = 0; i < g_lib_count; i++) {
        load_sections(g_loaded_libs[i], ET_DYN);
    }

    for (int i = 0; i < g_lib_count; i++) {
        file_relocate(g_loaded_libs[i]);
        init_lib(g_loaded_libs[i]);
    }

    load_sections(prog, ET_EXEC);
    dynamic_linker(prog);

    debug_printf (
        "Link time: %d ms", c_timer_get_ms() - start
    ) else if (args.bench) fd_printf(1,
        "Link time: %d ms\n", c_timer_get_ms() - start
    );

    int (*main)() = (int (*)(int, char **, char **)) ((Elf32_Ehdr *) prog->file)->e_entry;

    free(prog->sym_table);
    free(prog->file);
    free(prog->name);
    free(prog);

    dlfcn_error = 0;

    if (!args.bench) {
        ret = main(argc - args.arg_offset, argv + args.arg_offset, envp);
    }

    debug_printf("Exit with code %d", ret);

    while (g_lib_count) {
        if (g_loaded_libs[0]->ref_count > 1) {
            debug_printf("Unclosed library '%s'", g_loaded_libs[0]->name);
            g_loaded_libs[0]->ref_count = 1;
        }
        dlclose(g_loaded_libs[0]);
    }

    free(g_loaded_libs);

    if (g_cleanup) {
        int pid = c_process_get_pid();
        int leaks = c_mem_get_info(7, pid);

        debug_printf("Clean up %d alloc%s (%d bytes)",
            leaks,
            leaks == 1 ? "" : "s",
            c_mem_get_info(8, pid)
        );
        if (leaks > 0) {
            c_mem_free_all(pid);
        }
    }

    return ret;
}
