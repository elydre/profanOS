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

typedef struct {
    uint32_t d_tag;         // Entry type
    union {
        uint32_t d_val;     // Integer value
        uint32_t d_ptr;     // Address value
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

    uint8_t *mem;

    uint8_t *dymsym;
    uint8_t *dynstr;
} dl_t;

int is_valid_elf(void *data, uint16_t required_type) {
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)data;
    if (memcmp(ehdr->e_ident, (void *) ELFMAG, SELFMAG) != 0) {
        return 0;
    }
    if (ehdr->e_type != required_type) {
        return 0;
    }
    if (ehdr->e_machine != EM_386) {
        return 0;
    }
    return 1;
}

int load_sections(dl_t *dl, uint16_t type) {
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)dl->file;
    Elf32_Shdr *shdr = (Elf32_Shdr *)(dl->file + ehdr->e_shoff);

    uint32_t required_size = 0;

    for (int i = 0; i < ehdr->e_shnum; i++) {
        if (shdr[i].sh_type == SHT_PROGBITS &&
            shdr[i].sh_addr + shdr[i].sh_size > required_size
        ) required_size = shdr[i].sh_addr + shdr[i].sh_size;
    }
    required_size -= ehdr->e_entry;
    required_size = (required_size + 0xFFF) & ~0xFFF;

    printf("required_size: %x\n", required_size);

    if (type == ET_EXEC) {
        dl->mem = (void *) 0xC0000000;
        c_scuba_add(0xC0000000, required_size / 0x1000);
    } else {
        dl->mem = (void *) c_mem_alloc(required_size, 0x1000, 1);
    }

    for (int i = 0; i < ehdr->e_shnum; i++) {
        if (shdr[i].sh_type == SHT_PROGBITS) {
            // printf("loading section %d at %p\n", i, dl->mem + shdr[i].sh_addr);
            if (type == ET_EXEC) {
                printf("loading section %d at %p\n", i, shdr[i].sh_addr);
                memcpy((void *) shdr[i].sh_addr, dl->file + shdr[i].sh_offset, shdr[i].sh_size);
            }
            else
                memcpy(dl->mem + shdr[i].sh_addr, dl->file + shdr[i].sh_offset, shdr[i].sh_size);
        }
    }

    return 0;
}

int file_relocate(dl_t *dl) {
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)dl->file;
    Elf32_Shdr *shdr = (Elf32_Shdr *)(dl->file + ehdr->e_shoff);

    for (uint32_t i = 0; i < ehdr->e_shnum; i++) {
        if (shdr[i].sh_type == 9) { // SHT_REL
            printf("rel in section %d\n", i);
            Elf32_Rel *rel = (Elf32_Rel *)(dl->file + shdr[i].sh_offset);
            for (uint32_t j = 0; j < shdr[i].sh_size / sizeof(Elf32_Rel); j++) {
                // printf("rel[%d].r_offset = %p\n", j, dl->mem + rel[j].r_offset);
                *(uint32_t *)(dl->mem + rel[j].r_offset) += (uint32_t) dl->mem;
            }
        }
    }

    return 0;
}

void *open_elf(const char *filename, uint16_t required_type) {
    dl_t *dl = malloc(sizeof(dl_t));

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
    dl_t *dl = open_elf(filename, ET_DYN);
    load_sections(dl, ET_DYN);
    file_relocate(dl);
    puts("");

    return dl;
}

void *dlsym(void *handle, const char *symbol) {
    dl_t *dl = (dl_t *)handle;
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)dl->file;
    Elf32_Shdr *shdr = (Elf32_Shdr *)(dl->file + ehdr->e_shoff);

    for (uint8_t *p = dl->dymsym; p < dl->dymsym + shdr[1].sh_size; p += sizeof(Elf32_Sym)) {
        Elf32_Sym *sym = (Elf32_Sym *)p;
        if (strcmp((char *) dl->dynstr + sym->st_name, symbol) == 0) {
            printf("symbol found: %s at %p\n", symbol, dl->mem + sym->st_value);
            return (void *)(dl->mem + sym->st_value);
        }
    }

    printf("symbol not found: %s\n", symbol);
    return NULL;
}

int dlclose(void *handle) {
    dl_t *dl = (dl_t *)handle;
    free(dl->file);
    free(dl->mem);
    free(dl);
    return 0;
}

int dynamic_linker(dl_t *exec, dl_t *lib) {
    // resolve symbols in the executable using the library

    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)exec->file;
    Elf32_Shdr *shdr = (Elf32_Shdr *)(exec->file + ehdr->e_shoff);


    int size = 0;

    for (uint32_t i = 0; i < ehdr->e_shnum; i++) {
        if (shdr[i].sh_type == 9) { // SHT_REL
            printf("rel in section %d\n", i);
            Elf32_Rel *rel = (Elf32_Rel *)(exec->file + shdr[i].sh_offset);
            for (uint32_t j = 0; j < shdr[i].sh_size / sizeof(Elf32_Rel); j++) {
                char *name = (char *) exec->dynstr + ((Elf32_Sym *) exec->dymsym + ELF32_R_SYM(rel[j].r_info))->st_name;
                
                printf("rel[%d].r_offset = %p (%s)\n", j, rel[j].r_offset, name);
        
                void *sym = dlsym(lib, name);

                *(uint32_t *)(rel[j].r_offset) = (uint32_t) sym;
            }
        }
    }

    

    return 0;
}

int main(int c) {
    /*
    system("cc -d; tcc -shared /user/lib.c -o /user/libtest.so");

    void *handle = dlopen("/user/libtest.so", 42);
    if (handle == NULL) {
        fprintf(stderr, "dlopen failed...\n");
        return 1;
    }

    void (*set_val)(int) = dlsym(handle, "set_val");
    int (*get_val)() = dlsym(handle, "get_val");

    if (set_val == NULL || get_val == NULL) {
        fprintf(stderr, "dlsym failed...\n");
        return 1;
    }

    set_val(42);
    printf("get_val() = %d\n", get_val());

    dlclose(handle);
    */

    system(
        "cc -d;"
        "tcc -shared /user/lib.c -o /user/libtest.so;"
        "tcc -ltest /user/test.c -o /test.elf -L/user"
    );

    void *lib = dlopen("/user/libtest.so", 42);
    if (lib == NULL) {
        fprintf(stderr, "dlopen failed...\n");
        return 1;
    }

    dl_t *test = open_elf("/test.elf", ET_EXEC);
    if (test == NULL) {
        fprintf(stderr, "open_elf failed...\n");
        return 1;
    }

    load_sections(test, ET_EXEC);
    dynamic_linker(test, lib);

    int (*main)() = (int (*)()) ((Elf32_Ehdr *)test->file)->e_entry;
    printf("main = %p\n", main);
    printf("main() = %d\n", main());

    free(test->file);
    free(test);

    dlclose(lib);
    return 0;
}
