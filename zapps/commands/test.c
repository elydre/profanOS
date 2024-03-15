#include <stdlib.h>
#include <profan.h>
#include <string.h>
#include <stdio.h>

// elf header

// Elf32_Ehdr
typedef struct {
    unsigned char e_ident[16];    // ELF identification
    unsigned short e_type;        // Object file type
    unsigned short e_machine;    // Machine type
    unsigned int e_version;        // Object file version
    unsigned int e_entry;        // Entry point address
    unsigned int e_phoff;        // Program header offset
    unsigned int e_shoff;        // Section header offset
    unsigned int e_flags;        // Processor-specific flags
    unsigned short e_ehsize;        // ELF header size
    unsigned short e_phentsize;    // Size of program header entry
    unsigned short e_phnum;        // Number of program header entries
    unsigned short e_shentsize;    // Size of section header entry
    unsigned short e_shnum;        // Number of section header entries
    unsigned short e_shstrndx;    // Section name string table index
} Elf32_Ehdr;

// Elf32_Shdr
typedef struct {
    unsigned int sh_name;        // Section name (string tbl index)
    unsigned int sh_type;        // Section type
    unsigned int sh_flags;        // Section flags
    unsigned int sh_addr;        // Address where section is to be loaded
    unsigned int sh_offset;        // File offset of section data
    unsigned int sh_size;        // Size of section data
    unsigned int sh_link;        // Section index linked to this section
    unsigned int sh_info;        // Extra information
    unsigned int sh_addralign;    // Section alignment
    unsigned int sh_entsize;    // Entry size if section holds table
} Elf32_Shdr;

// dymsym
typedef struct {
    unsigned int st_name;        // Symbol name (string tbl index)
    unsigned int st_value;      // Symbol value
    unsigned int st_size;       // Symbol size
    unsigned char st_info;      // Symbol type and binding
    unsigned char st_other;     // Symbol visibility
    unsigned short st_shndx;    // Section index
} Elf32_Sym;

// dynstr
typedef struct {
    unsigned char *data;
} Elf32_Str;

#define ELFMAG          "\177ELF"
#define SELFMAG         4
#define ET_DYN          3
#define EM_386          3

typedef struct {
    uint32_t size;
    uint8_t *data;
} dl_t;

int is_valid_elf(void *data) {
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)data;
    if (memcmp(ehdr->e_ident, (void *) ELFMAG, SELFMAG) != 0) {
        return 0;
    }
    if (ehdr->e_type != ET_DYN) {
        return 0;
    }
    if (ehdr->e_machine != EM_386) {
        return 0;
    }
    return 1;
}

void *dlopen(const char *filename, int flag) {
    dl_t *dl = malloc(sizeof(dl_t));
    
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    dl->size = ftell(file);
    fseek(file, 0, SEEK_SET);
    dl->data = malloc(dl->size);

    fread(dl->data, 1, dl->size, file);
    fclose(file);

    if (dl->size < sizeof(Elf32_Ehdr) || !is_valid_elf(dl->data)) {
        free(dl->data);
        free(dl);
        return NULL;
    }

    return dl;
}

void *dlsym(void *handle, const char *symbol) {
    dl_t *dl = (dl_t *)handle;
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)dl->data;

    Elf32_Shdr *shdr = (Elf32_Shdr *)(dl->data + ehdr->e_shoff);
    uint8_t *dymsym = NULL;
    uint8_t *dynstr = NULL;

    for (int i = 0; i < ehdr->e_shnum; i++) {
        printf("shdr[%d].sh_type = %d\n", i, shdr[i].sh_type);
        if (shdr[i].sh_type == 11) {
            dymsym = dl->data + shdr[i].sh_offset;
            dynstr = dl->data + shdr[shdr[i].sh_link].sh_offset;
            break;
        }
    }

    if (dymsym == NULL || dynstr == NULL) {
        printf("dymsym or dynstr is NULL\n");
        return NULL;
    }

    printf("dymsym = %p, dynstr = %p\n", dymsym - dl->data, dynstr - dl->data);

    for (uint8_t *p = dymsym; p < dymsym + shdr[1].sh_size; p += sizeof(Elf32_Sym)) {
        Elf32_Sym *sym = (Elf32_Sym *)p;
        if (strcmp((char *) dynstr + sym->st_name, symbol) == 0) {
            printf("symbol found: %s\n", symbol);
            printf("sym->st_value = %p\n", sym->st_value);
            printf("sym->st_size = %d\n", sym->st_size);
            printf("sym->st_info = %d\n", sym->st_info);
            printf("sym->st_other = %d\n", sym->st_other);
            printf("sym->st_shndx = %d\n", sym->st_shndx);
            return (void *)((uint8_t *) sym->st_value + (uint32_t) dl->data - 0x1000);
        }
    }

    printf("symbol not found: %s\n", symbol);
    return NULL;
}

int dlclose(void *handle) {
    dl_t *dl = (dl_t *)handle;
    free(dl->data);
    free(dl);
    return 0;
}

int main(int c) {
    system("cc -d; tcc -shared /user/lib.c -o /user/libtest.so");
    if (c != 1) return 0;
    void *handle = dlopen("/user/libtest.so", 42);
    if (handle == NULL) {
        fprintf(stderr, "dlopen failed...\n");
        return 1;
    }

    int (*add)(int, int) = dlsym(handle, "add");

    printf("add(1, 2) = %d\n", add(1, 2));

    dlclose(handle);
    return 0;
}
