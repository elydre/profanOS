#include <syscall.h>
#include <filesys.h>
#include <libmmq.h>
#include <dlfcn.h>

#define raise_error(fmt, ...) do { fd_printf(2, "DELUGE FATAL: "fmt"\n", ##__VA_ARGS__); exit(1); } while (0)

// elf header

typedef struct {
    uint8_t  e_ident[16];   // ELF identification
    uint16_t e_type;        // Object file type
    uint16_t e_machine;     // Machine type
    uint32_t e_version;     // Object file version
    uint32_t e_entry;       // Entry point address
    uint32_t e_phoff;       // Program header offset
    uint32_t e_shoff;       // Section header offset
    uint32_t e_flags;       // Processor-specific flags
    uint16_t e_ehsize;      // ELF header size
    uint16_t e_phentsize;   // Size of program header entry
    uint16_t e_phnum;       // Number of program header entries
    uint16_t e_shentsize;   // Size of section header entry
    uint16_t e_shnum;       // Number of section header entries
    uint16_t e_shstrndx;    // Section name string table index
} Elf32_Ehdr;

typedef struct {
    uint32_t sh_name;       // Section name (string tbl index)
    uint32_t sh_type;       // Section type
    uint32_t sh_flags;      // Section flags
    uint32_t sh_addr;       // Address where section is to be loaded
    uint32_t sh_offset;     // File offset of section data
    uint32_t sh_size;       // Size of section data
    uint32_t sh_link;       // Section index linked to this section
    uint32_t sh_info;       // Extra information
    uint32_t sh_addralign;  // Section alignment
    uint32_t sh_entsize;    // Entry size if section holds table
} Elf32_Shdr;

typedef struct {
    uint32_t st_name;       // Symbol name (string tbl index)
    uint32_t st_value;      // Symbol value
    uint32_t st_size;       // Symbol size
    uint8_t  st_info;       // Symbol type and binding
    uint8_t  st_other;      // Symbol visibility
    uint16_t st_shndx;      // Section index
} Elf32_Sym;

typedef struct {
    uint32_t r_offset;      // Address
    uint32_t r_info;        // Relocation type and symbol index
} Elf32_Rel;

typedef struct {
    uint32_t d_tag;         // Entry type
    union {
        uint32_t d_val;      // Integer value
        uint32_t d_ptr;      // Address value
    } d_un;
} Elf32_Dyn;

typedef struct {
    uint32_t p_type;        // Segment type
    uint32_t p_offset;      // Segment offset
    uint32_t p_vaddr;       // Virtual address of segment
    uint32_t p_paddr;       // Physical address of segment
    uint32_t p_filesz;      // Size of segment in file
    uint32_t p_memsz;       // Size of segment in memory
    uint32_t p_flags;       // Segment flags
    uint32_t p_align;       // Segment alignment
} Elf32_Phdr;

#define ELFMAG          "\177ELF"
#define SELFMAG         4
#define ET_EXEC         2
#define ET_DYN          3
#define EM_386          3
#define SHT_PROGBITS    1

#define ELF32_R_SYM(i)  ((i) >> 8)
#define ELF32_R_TYPE(i) ((uint8_t)(i))

#define ELF32_ST_BIND(i) ((i) >> 4)

typedef struct {
    uint32_t size;
    uint8_t *file;
    char *name;

    uint8_t *mem;

    Elf32_Sym *dymsym;
    Elf32_Dyn *dynamic;
    char *dynstr;

    int dynsym_size;
} elfobj_t;

elfobj_t **g_loaded_libs;
int g_lib_count;
int g_list_deps;
int g_cleanup;

int is_valid_elf(void *data, uint16_t required_type) {
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)data;
    return !(
        memcmp(ehdr->e_ident, (void *) ELFMAG, SELFMAG) != 0 ||
        ehdr->e_type != required_type ||
        ehdr->e_machine != EM_386
    );
}

char *assemble_path(const char *dir, const char *file) {
    int len1 = strlen(dir);
    char *path = malloc(len1 + strlen(file) + 2);
    strcpy(path, dir);
    strcpy(path + len1, "/");
    strcpy(path + len1 + 1, file);
    return path;
}

char *get_full_path(const char *lib) {
    char *full_path;
    if (lib[0] == '/') {
        full_path = malloc(strlen(lib) + 1);
        strcpy(full_path, lib);
        return full_path;
    }
    full_path = assemble_path("/user", lib);
    if (!full_path || fu_is_file(fu_path_to_sid(ROOT_SID, full_path)))
        return full_path;
    free(full_path);
    full_path = assemble_path("/lib", lib);
    if (!full_path || fu_is_file(fu_path_to_sid(ROOT_SID, full_path)))
        return full_path;
    return NULL;
}

char **get_required_libs(elfobj_t *obj) {
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)obj->file;
    Elf32_Shdr *shdr = (Elf32_Shdr *)(obj->file + ehdr->e_shoff);

    char *tmp, **libs = NULL;
    int lib_count = 0;

    if (obj->dynamic == NULL) {
        raise_error("no dynamic section found in '%s'", obj->name);
        return NULL;
    }

    for (int i = 0; obj->dynamic[i].d_tag != 0; i++) {
        if (obj->dynamic[i].d_tag == 1) { // DT_NEEDED
            libs = realloc(libs, (lib_count + 2) * sizeof(char *));
            tmp = get_full_path((char *) obj->dynstr + obj->dynamic[i].d_un.d_val);
            if (tmp == NULL) {
                raise_error("library '%s' not found but required by '%s'", (char *) obj->dynstr + obj->dynamic[i].d_un.d_val, obj->name);
                return NULL;
            }
            libs[lib_count++] = tmp;
        }
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
        if (shdr[i].sh_type == SHT_PROGBITS) {
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
        if (shdr[i].sh_type == 9) { // SHT_REL
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
                        raise_error("symbol '%s' not found in '%s'", name, dl->name);
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

        else if (shdr[i].sh_type == 4) { // SHT_RELA
            raise_error("SHT_RELA is not supported but found in '%s'", dl->name);
        }
    }
    return 0;
}

void *open_elf(const char *filename, uint16_t required_type, int isfatal) {
    elfobj_t *obj = calloc(1, sizeof(elfobj_t));

    if (g_list_deps)
        fd_printf(2, "[deluge] Loading '%s'\n", filename);

    obj->name = get_full_path(filename);

    sid_t sid = fu_path_to_sid(ROOT_SID, obj->name);
    if (!fu_is_file(sid)) {
        if (isfatal)
            raise_error("'%s' not found", filename);
        free(obj->name);
        free(obj);
        return NULL;
    }

    obj->size = fu_get_file_size(sid);
    obj->file = malloc(obj->size);
    fu_file_read(sid, obj->file, 0, obj->size);

    if (obj->size < sizeof(Elf32_Ehdr) || !is_valid_elf(obj->file, required_type)) {
        if (isfatal)
            raise_error("'%s' is not a valid ELF file\n", filename);
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
                raise_error("no dynamic symbol table found in '%s'", filename);
            free(obj->file);
            free(obj->name);
            free(obj);
            return NULL;
        }
        return obj;
    }

    char **new_libs = get_required_libs(obj);

    if (new_libs == NULL)
        return obj;

    for (int i = 0; new_libs[i] != NULL; i++) {
        int found = 0;
        for (int j = 0; g_loaded_libs && j < g_lib_count; j++) {
            if (strcmp(new_libs[i], g_loaded_libs[j]->name) == 0) {
                found = 1;
                break;
            }
        }
        if (!found) {
            elfobj_t *lib = dlopen(new_libs[i], RTLD_FATAL);
            if (lib == NULL) {
                raise_error("dlopen failed for '%s'", new_libs[i]);
            }
            g_loaded_libs = realloc(g_loaded_libs, (g_lib_count + 1) * sizeof(elfobj_t *));
            g_loaded_libs[g_lib_count] = lib;
            g_lib_count++;
        }
        free(new_libs[i]);
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
    load_sections(dl, ET_DYN);
    file_relocate(dl);
    init_lib(dl);
    return dl;
}

void *dlsym(void *handle, const char *symbol) {
    if (handle == NULL) {
        dlfcn_error = 2;
        return NULL;
    }

    elfobj_t *dl = handle;

    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)dl->file;
    Elf32_Shdr *shdr = (Elf32_Shdr *)(dl->file + ehdr->e_shoff);

    if (!dl->dymsym) {
        dlfcn_error = 2;
        return NULL;
    }

    if (ehdr->e_type == ET_EXEC) {
        for (uint32_t i = 0; i < dl->dynsym_size / sizeof(Elf32_Sym); i++) {
            if (strcmp(dl->dynstr + dl->dymsym[i].st_name, symbol) == 0) {
                return (void *) dl->dymsym[i].st_value;
            }
        }
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

    for (uint32_t i = 0; i < dl->dynsym_size / sizeof(Elf32_Sym); i++) {
        if (strcmp(dl->dynstr + dl->dymsym[i].st_name, symbol) == 0) {
            return dl->mem + dl->dymsym[i].st_value;
        }
    }

    dlfcn_error = 2;
    return NULL;
}

int dlclose(void *handle) {
    if (handle == NULL)
        return 0;

    elfobj_t *dl = handle;
    fini_lib(dl);
    free(dl->file);
    free(dl->name);
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
        if (shdr[i].sh_type == 9) { // SHT_REL
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
                        raise_error("symbol '%s' not found", name);
                }
                switch (type) {
                    case R_386_JMP_SLOT:    // word32  S
                        *(uint32_t *)(rel[j].r_offset) = val;
                        break;
                    case R_386_GLOB_DAT:    // word32  S
                        *(uint32_t *)(rel[j].r_offset) = val;
                        break;
                    default:
                        raise_error("relocation type %d in '%s' not supported", type, exec->name);
                        break;
                }
            }
        }

        else if (shdr[i].sh_type == 4) { // SHT_RELA
            raise_error("SHT_RELA is not supported but found in '%s'", exec->name);
            while (1);
        }
    }

    return 0;
}

typedef struct {
    char *name;
    uint8_t bench;
    int arg_offset;
} deluge_args_t;

void show_help(int full) {
    fd_printf(1, "Usage: deluge [options] <file> [args]\n");
    if (!full) {
        fd_printf(1, "Try 'deluge -h' for more information.\n");
        return;
    }

    fd_printf(1, "Options:\n");
    fd_printf(1, "  -h  Show this help message\n");
    fd_printf(1, "  -b  Bench link time\n");
    fd_printf(1, "  -e  use filename as argument\n");
    fd_printf(1, "  -l  List dependencies\n");
}

deluge_args_t deluge_parse(int argc, char **argv) {
    deluge_args_t args;
    args.name = NULL;
    args.bench = 0;
    g_list_deps = 0;

    int move_arg = 1;

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (argv[i][1] == 'h') {
                show_help(1);
                exit(0);
            } else if (argv[i][1] == 'b') {
                args.bench = 1;
            } else if (argv[i][1] == 'e') {
                move_arg = 0;
            } else if (argv[i][1] == 'l') {
                g_list_deps = 1;
            } else {
                fd_printf(2, "Unknown option: %s\n", argv[i]);
                show_help(0);
                exit(1);
            }
        } else {
            args.name = argv[i];
            args.arg_offset = i + move_arg;
            break;
        }
    }

    if (args.name == NULL) {
        fd_printf(2, "No file specified\n");
        show_help(0);
        exit(1);
    }

    return args;
}

int main(int argc, char **argv, char **envp) {
    g_loaded_libs = NULL;
    g_lib_count = 0;
    g_cleanup = 0;

    deluge_args_t args = deluge_parse(argc, argv);
    uint32_t start;

    if (args.bench) {
        start = c_timer_get_ms();
    }

    elfobj_t *test = open_elf(args.name, ET_EXEC, 1);
    if (test == NULL) {
        raise_error("failed to open '%s'", args.name);
        return 1;
    }

    load_sections(test, ET_EXEC);
    dynamic_linker(test);

    if (args.bench) {
        fd_printf(2, "Link time: %d ms\n", c_timer_get_ms() - start);
    }

    int (*main)() = (int (*)(int, char **, char **)) ((Elf32_Ehdr *) test->file)->e_entry;

    free(test->file);
    free(test->name);
    free(test);

    dlfcn_error = 0;

    int ret = main(argc - args.arg_offset, argv + args.arg_offset, envp);

    for (int i = 0; i < g_lib_count; i++)
        dlclose(g_loaded_libs[i]);
    free(g_loaded_libs);

    if (g_cleanup)
        c_mem_free_all(c_process_get_pid());

    return ret;
}
