/*****************************************************************************\
|   === dlgext.h : 2024 ===                                                   |
|                                                                             |
|    Header for the deluge linker and its extension                .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef DLGEXT_ID
#define DLGEXT_ID 1007

/*********************
 *                  *
 *    ELF header    *
 *                  *
 ********************/

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

/******************************
 *                           *
 *    Types and ext funcs    *
 *                           *
 *****************************/

typedef struct {
    const char *key;
    Elf32_Sym *data;
    uint32_t hash;
    void *next;
} dlg_hash_t;

typedef struct {
    uint32_t size;
    uint8_t *file;
    char *name;
    int ref_count;
    int need_free;

    uint8_t *mem;

    Elf32_Sym *sym_tab;
    uint32_t sym_size;
    char *sym_str;

    Elf32_Sym *dym_tab;
    uint32_t dym_size;
    char *dym_str;

    Elf32_Dyn *dynamic;
    dlg_hash_t *hash_table;
} elfobj_t;

#ifndef DLGEXT_C
#define get_func_addr ((uint32_t (*)(uint32_t, uint32_t)) *(uint32_t *) 0x1ffffb)
#define dlgext_libc ((elfobj_t *(*)(void)) get_func_addr(DLGEXT_ID, 2))
#endif

#endif
