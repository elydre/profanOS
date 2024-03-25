#include <syscall.h>
#include <filesys.h>
#include <libmmq.h>

#define raise_error(fmt, ...) do { fd_printf(2, "DELUGE FATAL: "fmt, ##__VA_ARGS__); exit(1); } while (0)

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
    uint32_t r_offset;      // Address
    uint32_t r_info;        // Relocation type and symbol index
    int32_t r_addend;       // Addend
} Elf32_Rela;

typedef struct {
    uint32_t d_tag;         // Entry type
    union {
        uint32_t d_val;      // Integer value
        uint32_t d_ptr;      // Address value
    } d_un;
} Elf32_Dyn;

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
    char *dynstr;

    int dynsym_size;
} elfobj_t;

elfobj_t **g_loaded_libs;
int g_lib_count;

void *dlsym(void *handle, const char *symbol);
void *dlopen(const char *filename, int flag);

int is_valid_elf(void *data, uint16_t required_type) {
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)data;
    return !(
        memcmp(ehdr->e_ident, (void *) ELFMAG, SELFMAG) != 0 ||
        ehdr->e_type != required_type ||
        ehdr->e_machine != EM_386
    );
}

char *assemble_path(char *dir, char *file) {
    int len1 = strlen(dir);
    char *path = malloc(len1 + strlen(file) + 2);
    strcpy(path, dir);
    strcpy(path + len1, "/");
    strcpy(path + len1 + 1, file);
    return path;
}

char *get_full_path(char *lib) {
    char *full_path;
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
    Elf32_Dyn *dyn = NULL;

    char *tmp, **libs = NULL;
    int lib_count = 0;

    for (int i = 0; i < ehdr->e_shnum; i++) {
        if (shdr[i].sh_type == 6) { // SHT_DYNAMIC
            dyn = (Elf32_Dyn *)(obj->file + shdr[i].sh_offset);
            break;
        }
    }

    if (dyn == NULL) {
        raise_error("no dynamic section found in '%s'\n", obj->name);
        return NULL;
    }

    for (int i = 0; dyn[i].d_tag != 0; i++) {
        if (dyn[i].d_tag == 1) { // DT_NEEDED
            libs = realloc(libs, (lib_count + 2) * sizeof(char *));
            tmp = get_full_path((char *) obj->dynstr + dyn[i].d_un.d_val);
            if (tmp == NULL) {
                raise_error("library '%s' not found but required by '%s'\n", (char *) obj->dynstr + dyn[i].d_un.d_val, obj->name);
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

int load_sections(elfobj_t *obj, uint16_t type) {
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)obj->file;
    Elf32_Shdr *shdr = (Elf32_Shdr *)(obj->file + ehdr->e_shoff);

    uint32_t required_size = 0;

    for (int i = 0; i < ehdr->e_shnum; i++) {
        if (shdr[i].sh_type == SHT_PROGBITS && shdr[i].sh_flags & 2) {
            if (shdr[i].sh_addr + shdr[i].sh_size > required_size)
                required_size = shdr[i].sh_addr + shdr[i].sh_size;
        }
    }

    if (type == ET_EXEC)
        required_size -= 0xC0000000;
    required_size = (required_size + 0xFFF) & ~0xFFF;

    if (type == ET_EXEC) {
        obj->mem = (void *) 0xC0000000;
        c_scuba_add(0xC0000000, required_size / 0x1000);
    } else {
        obj->mem = (void *) c_mem_alloc(required_size, 0x1000, 1);
    }
    memset(obj->mem, 0, required_size);

    for (int i = 0; i < ehdr->e_shnum; i++) {
        if (shdr[i].sh_type == SHT_PROGBITS && shdr[i].sh_flags & 2) {
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
                    val = (uint32_t) dlsym(dl, name);
                    if (val == 0) {
                        raise_error("symbol '%s' not found in '%s'\n", name, dl->name);
                    }
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
                        raise_error("relocation type %d in '%s' not supported\n", type, dl->name);
                        break;
                }
            }
        }

        else if (shdr[i].sh_type == 4) { // SHT_RELA
            raise_error("SHT_RELA is not supported but found in '%s'\n", dl->name);
        }
    }
    return 0;
}

void *open_elf(const char *filename, uint16_t required_type) {
    elfobj_t *obj = malloc(sizeof(elfobj_t));
    memset(obj, 0, sizeof(elfobj_t));
    fd_printf(2, "loading: %s\n", filename);

    sid_t sid = fu_path_to_sid(ROOT_SID, (void *) filename);
    if (!fu_is_file(sid)) {
        raise_error("'%s' not found\n", filename);
        free(obj);
        return NULL;
    }

    obj->size = fu_get_file_size(sid);
    obj->file = malloc(obj->size);
    fu_file_read(sid, obj->file, 0, obj->size);

    if (obj->size < sizeof(Elf32_Ehdr) || !is_valid_elf(obj->file, required_type)) {
        raise_error("'%s' is not a valid ELF file\n", filename);
        free(obj->file);
        free(obj);
        return NULL;
    }

    obj->name = malloc(strlen(filename) + 1);
    strcpy(obj->name, filename);

    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)obj->file;
    Elf32_Shdr *shdr = (Elf32_Shdr *)(obj->file + ehdr->e_shoff);

    for (int i = 0; i < ehdr->e_shnum; i++) {
        if (shdr[i].sh_type == 11) { // SHT_DYNSYM
            obj->dymsym = (Elf32_Sym *)(obj->file + shdr[i].sh_offset);
            obj->dynstr = (char *) obj->file + shdr[shdr[i].sh_link].sh_offset;
            obj->dynsym_size = shdr[i].sh_size;
        }

        /*
        if (shdr[i].sh_type == 2) { // SHT_SYMTAB
            obj->symtab = obj->file + shdr[i].sh_offset;
            obj->strtab = obj->file + shdr[shdr[i].sh_link].sh_offset;
            obj->symtab_size = shdr[i].sh_size;
        }*/
    }

    if (obj->dymsym == NULL) {
        if (required_type == ET_DYN) {
            raise_error("no dynamic symbol table found in '%s'\n", filename);
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
            elfobj_t *lib = dlopen(new_libs[i], ET_DYN);
            if (lib == NULL) {
                raise_error("dlopen failed for '%s'\n", new_libs[i]);
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

void *dlopen(const char *filename, int flag) {
    elfobj_t *dl = open_elf(filename, ET_DYN);
    if (dl == NULL) {
        return NULL;
    }
    load_sections(dl, ET_DYN);
    file_relocate(dl);
    return dl;
}

void *dlsym(void *handle, const char *symbol) {
    elfobj_t *dl = (elfobj_t *)handle;
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)dl->file;
    Elf32_Shdr *shdr = (Elf32_Shdr *)(dl->file + ehdr->e_shoff);

    if (ehdr->e_type != ET_DYN || !dl->dymsym) {
        return NULL;
    }

    for (uint32_t i = 0; i < dl->dynsym_size / sizeof(Elf32_Sym); i++) {
        if (strcmp(dl->dynstr + dl->dymsym[i].st_name, symbol) == 0) {
            return dl->mem + dl->dymsym[i].st_value;
        }
    }

    return NULL;
}

int init_lib(elfobj_t *lib) {
    // call constructors

    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)lib->file;
    Elf32_Shdr *shdr = (Elf32_Shdr *)(lib->file + ehdr->e_shoff);

    Elf32_Dyn *dyn;

    void (**init_array)(void) = NULL;
    int size = 0;

    for (int i = 0; i < ehdr->e_shnum; i++) {
        if (shdr[i].sh_type == 6) { // SHT_DYNAMIC
            dyn = (Elf32_Dyn *)(lib->file + shdr[i].sh_offset);
            for (int j = 0; dyn[j].d_tag != 0; j++) {
                if (dyn[j].d_tag == 25) { // DT_INIT
                    init_array = (void (**)(void)) (lib->mem + dyn[j].d_un.d_ptr);
                }
                if (dyn[j].d_tag == 27) { // DT_INIT_ARRAYSZ
                    size = dyn[j].d_un.d_val / sizeof(void *);
                }
            }
        }
    }

    if (init_array == NULL) {
        return 0;
    }

    for (int i = 0; i < size; i++) {
        init_array[i]();
    }

    return 0;
}

int dlclose(void *handle) {
    elfobj_t *dl = (elfobj_t *) handle;
    free(dl->file);
    free(dl->name);
    free(dl->mem);
    free(dl);
    return 0;
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
                    for (int k = 0; k < g_lib_count && val == 0; k++) {
                        val = (uint32_t) dlsym(g_loaded_libs[k], name);
                        break;
                    }
                    if (val == 0) {
                        raise_error("symbol '%s' not found in any library\n", name);
                    }
                }
                switch (type) {
                    case R_386_JMP_SLOT:    // word32  S
                        *(uint32_t *)(rel[j].r_offset) = val;
                        break;
                    case R_386_GLOB_DAT:    // word32  S
                        *(uint32_t *)(rel[j].r_offset) = val;
                        break;
                    default:
                        raise_error("relocation type %d in '%s' not supported\n", type, exec->name);
                        break;
                }
            }
        }

        else if (shdr[i].sh_type == 4) { // SHT_RELA
            raise_error("SHT_RELA is not supported but found in '%s'\n", exec->name);
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
}

deluge_args_t deluge_parse(int argc, char **argv) {
    deluge_args_t args;
    args.name = NULL;
    args.bench = 0;

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

    deluge_args_t args = deluge_parse(argc, argv);
    uint32_t start;

    if (args.bench) {
        start = c_timer_get_ms();
    }

    elfobj_t *test = open_elf(args.name, ET_EXEC);
    if (test == NULL) {
        raise_error("failed to open '%s'\n", args.name);
        return 1;
    }

    for (int i = 0; i < g_lib_count; i++) {
        init_lib(g_loaded_libs[i]);
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

    int ret = main(argc - args.arg_offset, argv + args.arg_offset, envp);

    for (int i = 0; i < g_lib_count; i++) {
        dlclose(g_loaded_libs[i]);
    }
    free(g_loaded_libs);

    return ret;
}
