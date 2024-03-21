#include <syscall.h>
#include <stdlib.h>
#include <profan.h>
#include <string.h>
#include <stdio.h>

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

    uint8_t *mem;

    uint8_t *dymsym;
    uint8_t *dynstr;
} elfobj_t;

void *dlsym(void *handle, const char *symbol);

int is_valid_elf(void *data, uint16_t required_type) {
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)data;
    return !(
        memcmp(ehdr->e_ident, (void *) ELFMAG, SELFMAG) != 0 ||
        ehdr->e_type != required_type ||
        ehdr->e_machine != EM_386
    );
}

int load_sections(elfobj_t *obj, uint16_t type) {
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)obj->file;
    Elf32_Shdr *shdr = (Elf32_Shdr *)(obj->file + ehdr->e_shoff);

    uint32_t required_size = 0;

    for (int i = 0; i < ehdr->e_shnum; i++) {
        if (shdr[i].sh_type == SHT_PROGBITS) {
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

    uint32_t val;
    uint8_t type;
    char *name;

    for (uint32_t i = 0; i < ehdr->e_shnum; i++) {
        if (shdr[i].sh_type == 9) { // SHT_REL
            Elf32_Rel *rel = (Elf32_Rel *)(dl->file + shdr[i].sh_offset);
            for (uint32_t j = 0; j < shdr[i].sh_size / sizeof(Elf32_Rel); j++) {
                name = (char *) dl->dynstr + ((Elf32_Sym *) dl->dymsym + ELF32_R_SYM(rel[j].r_info))->st_name;
                val = 0;
                type = ELF32_R_TYPE(rel[j].r_info);
                if (does_type_required_sym(type)) {
                    val = (uint32_t) dlsym(dl, name);
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
                    default:
                        printf("unsupported relocation type: %d\n", type);
                        while (1);
                        break;
                }
            }
        }
    }
    return 0;
}

void *open_elf(const char *filename, uint16_t required_type) {
    elfobj_t *dl = malloc(sizeof(elfobj_t));

    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        printf("file not found: %s\n", filename);
        free(dl);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    dl->size = ftell(file);
    fseek(file, 0, SEEK_SET);
    dl->file = malloc(dl->size);

    fread(dl->file, 1, dl->size, file);
    fclose(file);

    if (dl->size < sizeof(Elf32_Ehdr) || !is_valid_elf(dl->file, required_type)) {
        printf("invalid elf file\n");
        free(dl->file);
        free(dl);
        return NULL;
    }

    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)dl->file;
    Elf32_Shdr *shdr = (Elf32_Shdr *)(dl->file + ehdr->e_shoff);

    for (int i = 0; i < ehdr->e_shnum; i++) {
        if (shdr[i].sh_type == 11) {
            dl->dymsym = dl->file + shdr[i].sh_offset;
            dl->dynstr = dl->file + shdr[shdr[i].sh_link].sh_offset;
            break;
        }
    }

    if (dl->dymsym == NULL || dl->dynstr == NULL) {
        printf("no dynamic symbols found\n");
        free(dl->file);
        free(dl);
        return NULL;
    }

    return dl;
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

    for (uint8_t *p = dl->dymsym; p < dl->dymsym + shdr[1].sh_size; p += sizeof(Elf32_Sym)) {
        Elf32_Sym *sym = (Elf32_Sym *)p;
        if (strcmp((char *) dl->dynstr + sym->st_name, symbol) == 0) {
            return (void *)(dl->mem + sym->st_value);
        }
    }

    return NULL;
}

int dlclose(void *handle) {
    elfobj_t *dl = (elfobj_t *) handle;
    free(dl->file);
    free(dl->mem);
    free(dl);
    return 0;
}

int dynamic_linker(elfobj_t *exec, elfobj_t *lib) {
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)exec->file;
    Elf32_Shdr *shdr = (Elf32_Shdr *)(exec->file + ehdr->e_shoff);

    int size = 0;
    uint8_t type;

    for (uint32_t i = 0; i < ehdr->e_shnum; i++) {
        if (shdr[i].sh_type == 9) { // SHT_REL
            Elf32_Rel *rel = (Elf32_Rel *)(exec->file + shdr[i].sh_offset);
            for (uint32_t j = 0; j < shdr[i].sh_size / sizeof(Elf32_Rel); j++) {
                char *name = (char *) exec->dynstr + ((Elf32_Sym *) exec->dymsym + ELF32_R_SYM(rel[j].r_info))->st_name;

                type = ELF32_R_TYPE(rel[j].r_info);
                if (type != R_386_JMP_SLOT) {
                    printf("unsupported relocation type: %d\n", type);
                    while (1);
                }

                void *sym = dlsym(lib, name);
                if (sym == NULL) {
                    printf("symbol not found: %s\n", name);
                    while (1);
                }
                *(uint32_t *)(rel[j].r_offset) = (uint32_t) sym;
            }
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
    puts("Usage: deluge [options] <file> [args]");
    if (!full) {
        puts("Try 'deluge -h' for more information.");
        return;
    }

    puts("Options:");
    puts("  -h  Show this help message");
    puts("  -b  Bench link time");
}

deluge_args_t deluge_parse(int argc, char **argv) {
    deluge_args_t args;
    args.name = NULL;
    args.bench = 0;

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (argv[i][1] == 'h') {
                show_help(1);
                exit(0);
            } else if (argv[i][1] == 'b') {
                args.bench = 1;
            } else {
                printf("Unknown option: %s\n", argv[i]);
                show_help(0);
                exit(1);
            }
        } else {
            args.name = argv[i];
            args.arg_offset = i + 1;
            break;
        }
    }

    if (args.name == NULL) {
        puts("No file specified.");
        show_help(0);
        exit(1);
    }

    return args;
}

int main(int argc, char **argv) {
    deluge_args_t args = deluge_parse(argc, argv);
    uint32_t start;

    if (args.bench) {
        start = c_timer_get_ms();
    }

    void *lib = dlopen("/user/libtest.so", 42);
    if (lib == NULL) {
        fprintf(stderr, "dlopen failed...\n");
        return 1;
    }

    elfobj_t *test = open_elf(args.name, ET_EXEC);
    if (test == NULL) {
        fprintf(stderr, "open_elf failed...\n");
        return 1;
    }

    load_sections(test, ET_EXEC);
    dynamic_linker(test, lib);

    if (args.bench) {
        printf("Link time: %d ms\n", c_timer_get_ms() - start);
    }

    int (*main)() = (int (*)()) ((Elf32_Ehdr *)test->file)->e_entry;
    main();

    free(test->file);
    free(test);

    dlclose(lib);
    return 0;
}
