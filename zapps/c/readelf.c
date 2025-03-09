/*****************************************************************************\
|   === readelf.c : 2024 ===                                                  |
|                                                                             |
|    Unix command implementation - print ELF file information      .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS - under GPLv3                  q. /|\  "   |
|    Original elf reader by tomcat1102                             `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <elf.h>

#define READELF_USAGE "Usage: readelf [options] <file1>\n"
#define READELF_HELPF "Try 'readelf -h' for more information.\n"

const char *g_filename;
char *g_filecnt;

typedef int (*proc)(void);

// procedure declarations
int display_dynamic_symbols(void);  // D
int print_help(void);               // h
int display_file_header(void);      // H
int display_needed_libraries(void); // n
int display_dynamic_reloc(void);    // R
int display_section_headers(void);  // S
int display_symbol_table(void);     // s

// procedure indexes in the following 'option_xxx' arrays
#define OPT_h 1
#define OPT_H 2

static const char *option_chars = "DhHnRSs";

static const proc option_procs[] = {
    display_dynamic_symbols, print_help, display_file_header,
    display_needed_libraries, display_dynamic_reloc,
    display_section_headers, display_symbol_table, NULL
};

#define PROC_COUNT (sizeof(option_procs) / sizeof(proc))
// invoke produces in 'procs' if not NULL, after options and files have been parsed
proc procs[PROC_COUNT] = {NULL};

int print_help(void) {
    puts(READELF_USAGE
        "Options:\n"
        "  -D   dynamic symbol table\n"
        "  -h   show this help page\n"
        "  -H   ELF file header\n"
        "  -n   needed libraries\n"
        "  -R   dynamic relocations\n"
        "  -S   sections header\n"
        "  -s   symbol table"
    );
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fputs(READELF_USAGE READELF_HELPF, stderr);
        return 1;
    } else if (argc == 2) {
        procs[OPT_H] = display_file_header;
    }

    int arg_i;

    // parse options
    for (arg_i = 1; arg_i < argc; arg_i++) {
        const char *arg = argv[arg_i];
        int option_idx = 0;

        if (arg[0] != '-')
            break;

        if (arg[1] == '-') {
            fprintf(stderr, "readelf: unrecognized option '%s'\n", arg);
            fputs(READELF_HELPF, stderr);
            return 1;
        }

        char *p_char;

        if (strlen(arg) != 2 || (p_char = strchr(option_chars, arg[1])) == NULL) {
            fprintf(stderr, "readelf: invalid option -- '%s'\n", arg + 1);
            fputs(READELF_HELPF, stderr);
            return 1;
        }

        option_idx = p_char - option_chars;

        procs[option_idx] = option_procs[option_idx];
    }

    if (procs[OPT_h] != NULL) {
        procs[OPT_h]();
        return 0;
    }

    if (arg_i != argc - 1) {
        fputs(READELF_USAGE READELF_HELPF, stderr);
        return 1;
    }

    // read and process input file
    g_filename = argv[arg_i];

    // open file for reading
    FILE *fp = fopen(g_filename, "r");
    if (fp == NULL) {
        fprintf(stderr, "readelf: %s: %m\n", g_filename);
        return 1;
    }

    // get file size and read the whole file
    fseek(fp, 0, SEEK_END);
    size_t file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if ((g_filecnt = malloc(file_size)) == NULL || fread(g_filecnt, 1, file_size, fp) != file_size) {
        fprintf(stderr, "readelf: %s: failed to read file\n", g_filename);
        fclose(fp);
        return 1;
    }
    fclose(fp);

    // check for ELF magic number
    Elf32_Ehdr *p_header = (Elf32_Ehdr *) g_filecnt;
    if (memcmp(p_header->e_ident, ELFMAG, SELFMAG) != 0) {
        fprintf(stderr, "readelf: %s: not an ELF file\n", g_filename);
        free(g_filecnt);
        return 1;
    }

    // check for elf class. now only ELF32 is supported. TODO we shall support ELF64 header later
    if (p_header->e_ident[EI_CLASS] != ELFCLASS32) {
        fprintf(stderr, "readelf: %s: unsupported ELF class\n", g_filename);
        free(g_filecnt);
        return 1;
    }

    // now invoke each procedure in proc array
    for (uint32_t i = 0; i < PROC_COUNT; i++) {
        if (procs[i] == NULL)
            continue;

        if (procs[i]() == 1) {
            fprintf(stderr, "readelf: %s: failed to process file\n", g_filename);
            break;
        }
    }
    free(g_filecnt);

    return 0;
}

// procedures for each readelf options
int display_file_header(void) {
    const Elf32_Ehdr *header = (Elf32_Ehdr*)g_filecnt;

    // Strings for meanings for some of the header fields.
    static char *elf_classes[ELFCLASSNUM] = {"invalid", "ELF32", "ELF64"};
    static char *elf_endians[ELFDATANUM] = {"invalid",
            "2's complement, little endian", "2's complement, big endian"};

    static char *elf_os[256] = {NULL};
    elf_os[ELFOSABI_NONE] = "Unix System V ABI";
    elf_os[ELFOSABI_HPUX] = "HP-UX";
    elf_os[ELFOSABI_NETBSD] = "NetBSD";
    elf_os[ELFOSABI_GNU] = "Object uses GNU ELF extensions";
    elf_os[ELFOSABI_SOLARIS] = "Sun Solaris";
    elf_os[ELFOSABI_AIX] = "IBM AIX";
    elf_os[ELFOSABI_IRIX] = "SGI Irix";
    elf_os[ELFOSABI_FREEBSD] = "FreeBSD";
    elf_os[ELFOSABI_TRU64] = "Compaq TRU64 UNIX";
    elf_os[ELFOSABI_MODESTO] = "Novell Modesto";
    elf_os[ELFOSABI_OPENBSD] = "OpenBSD";
    elf_os[ELFOSABI_ARM_AEABI] = "ARM EABI";
    elf_os[ELFOSABI_ARM] = "ARM";
    elf_os[ELFOSABI_STANDALONE] = "Standalone (embedded) application";

    static char *elf_types[ET_NUM] = { "NO TYPE", "REL", "EXE", "SHARED", "CORE"};
    static char *elf_machines[EM_NUM] = {NULL};
    elf_machines[EM_NONE] = "No machine";
    elf_machines[EM_SPARC] = "SUN SPARC";
    elf_machines[EM_386] = "Intel 80386";
    elf_machines[EM_860] = "Intel 80860";
    elf_machines[EM_PPC] = "PowerPC";
    elf_machines[EM_ARM] = "ARM";
    elf_machines[EM_X86_64] = "AMD x86-64 architecture";
    // the list goes on... See elf.h for more info

    // All ok. now print header info
    printf("Elf Header:\n");
    // Header identification. Magic number and other iWarningnfo
    printf(" off len  field: value\n");
    printf("  %02d -%2d  Magic: .%3.3s\n", EI_MAG0, SELFMAG, (char*)header + 1);
    printf("  %02d -%2d  Class: %s\n", EI_CLASS, 1, elf_classes[header->e_ident[EI_CLASS]]);
    printf("  %02d -%2d  Data : %s\n", EI_DATA, 1, elf_endians[header->e_ident[EI_DATA]]);
    printf("  %02d -%2d  Version: %d (current)\n", EI_VERSION, 1, header->e_ident[EI_VERSION]);
    printf("  %02d -%2d  OS/ABI: %s\n", EI_OSABI, 1, elf_os[header->e_ident[EI_OSABI]] ? : "unknown");
    printf("  %02d -%2d  ABI Version: %d\n", EI_ABIVERSION, 1, header->e_ident[EI_ABIVERSION]);
    printf("  %02d -%2d  Padding...\n", EI_PAD, EI_NIDENT - 1 - EI_PAD);

    size_t idx = EI_NIDENT;
    size_t field_size;

    // Object file type
    field_size = sizeof(header->e_type);
    printf("  %02ld -%2ld  Type: %s\n", idx, field_size, elf_types[header->e_type]);
    idx += field_size;

    // Architecture
    field_size = sizeof(header->e_machine);
    printf("  %02ld -%2ld  Arch: %s\n", idx, field_size, elf_machines[header->e_machine] ? : "unknown");
    idx += field_size;

    // Object file version
    field_size = sizeof(header->e_version);
    printf("  %02ld -%2ld  Version: %d (same as the above)\n", idx, field_size, header->e_version);
    idx += field_size;

    // Entry point virtual address
    field_size = sizeof(header->e_entry);
    printf("  %02ld -%2ld  Entry: 0x%08x\n", idx, field_size, header->e_entry);
    idx += field_size;

    // Program header table file offset
    field_size = sizeof(header->e_phoff);
    printf("  %02ld -%2ld  Start of program header: %d (bytes into file)\n", idx, field_size, header->e_phoff);
    idx += field_size;

    // Section header table file offset
    printf("  %02ld -%2ld  Start of section header: %d (bytes into file)\n", idx, field_size, header->e_shoff);
    idx += field_size;

    // Processor-specific flags
    printf("  %02ld -%2ld  Flags: 0x%x\n", idx, field_size, header->e_flags);
    idx += field_size;

    // ELF header size in bytes
    printf("  %02ld -%2ld  Size of this header: %d (bytes)\n", idx, field_size, header->e_ehsize);
    idx += field_size;

    // Program header table entry size
    printf("  %02ld -%2ld  Size of program headers: %d (bytes)\n", idx, field_size, header->e_phentsize);
    idx += field_size;

    // Program header table entry count
    printf("  %02ld -%2ld  Number of program headers: %d\n", idx, field_size, header->e_phnum);
    idx += field_size;

    // Section header table entry count
    printf("  %02ld -%2ld  Size of section headers: %d (bytes)\n", idx, field_size, header->e_shentsize);
    idx += field_size;

    // Section header table
    printf("  %02ld -%2ld  Number of section headers: %d\n", idx, field_size, header->e_shnum);
    idx += field_size;

    // Section header string table index
    printf("  %02ld -%2ld  Section number string table index: %d\n", idx, field_size, header->e_shstrndx);
    idx += field_size;

    return 0;
}

int display_section_headers(void) {
    Elf32_Ehdr *eh      = (Elf32_Ehdr *) g_filecnt;
    Elf32_Shdr *sh_base = (Elf32_Shdr *)(g_filecnt + eh->e_shoff);
    Elf32_Shdr *str_sh  = sh_base + eh->e_shstrndx;

    char *str_table = (char*)(g_filecnt + str_sh->sh_offset);

    // type string table
    static char *sh_types[SHT_NUM] = {"NULL", "PROGBITS", "SYMTAB", "STRTAB", "RELA", "HASH",
        "DYNAMIC", "NOTE", "NOBITS", "REL", "SHLIB", "DYNSYM", "NULL", "NULL", "INIT_ARRAY",
        "FINI_ARRAY", "PREINIT_ARRAY", "GROUP", "SYMTAB_SHNDX"};

    // flag characters set
    static char *sh_flags = "WAX MSILOGTCx"; // TODO support o, E, p flags

    // display section header infos
    if (procs[OPT_h] == NULL)
        printf("There are %d section headers, starting at offset 0x%x\n", eh->e_shnum, eh->e_shoff);

    printf("\nSection Headers:\n");    // Note 'ES' is entry size for some sections
    printf(" [Nr] %-17s %-15s %-8s %-6s %-6s %s %s %s %s %s\n",
            "Name", "Type", "Addr", "Off", "Size", "ES", "Flg", "LK", "Inf", "Al");

    for(int i = 0; i < eh->e_shnum; i++) {
        Elf32_Shdr *sh = sh_base + i;

        // parse flags
        char flags[sizeof(sh_flags)];

        unsigned int flag_bits = sh->sh_flags;
        int flag_num = sizeof(sh_flags) - 1;
        int flag_cnt = 0;

        int flag_i = 0;
        while (flag_i < flag_num) {
            if (flag_bits & 1)
                flags[flag_cnt++] = sh_flags[flag_i];
            flag_bits = flag_bits >> 1;
            flag_i++;
        }
        flags[flag_cnt] = '\0';

        printf(" [%2d] %-17.17s %-15s %-8.8x %6.6x %6.6x %2.2x %3s %2d %3d %2d\n",
                i,
                str_table + sh->sh_name,
                sh->sh_type < SHT_NUM ? sh_types[sh->sh_type] : "???",
                sh->sh_addr,
                sh->sh_offset,
                sh->sh_size,
                sh->sh_entsize,
                flags,
                sh->sh_link,
                sh->sh_info,
                sh->sh_addralign
        );
    }

    // print flags explanation
    printf("Key to Flags:\n"
            "W (write), A (alloc), X (execute), M (merge), S (strings), I (info),\n"
            "L (link order), O (extra OS processing required), G (group), T (TLS),\n"
            "C (compressed), x (unknown), o (OS specific), E (exclude)\n"
            "p (processor specific)\n\n"
    );

    return 0;
}

int display_symbols(uint16_t sh_type) {
    Elf32_Ehdr *eh       = (Elf32_Ehdr *) g_filecnt;
    Elf32_Shdr *sh_base  = (Elf32_Shdr *) (g_filecnt + eh->e_shoff);
    Elf32_Shdr *shstr_sh = sh_base + eh->e_shstrndx;
    Elf32_Shdr *sym_sh   = NULL;

    // get symbol table
    for (int i = 0; i < eh->e_shnum; i++) {
        Elf32_Shdr *sh = sh_base + i;
        if (sh->sh_type == sh_type) {
            sym_sh = sh;
            break;
        }
    }

    if (sym_sh == NULL) {
        fprintf(stderr, "readelf: %s: no symbol table\n", g_filename);
        return 1;
    }

    Elf32_Sym *symbol_table = (Elf32_Sym *) (g_filecnt + sym_sh->sh_offset);
    Elf32_Sym *symbol = NULL;

    // prepare string tables
    char *str_table   = (char *) (g_filecnt + (sh_base + sym_sh->sh_link)->sh_offset);
    char *shstr_table = (char *) (g_filecnt + shstr_sh->sh_offset);

    // print symbol table header
    const int symbol_cnt = sym_sh->sh_size / sizeof(Elf32_Sym);
    printf("Symbol table '%s' contains %d entries:\n", shstr_table + sym_sh->sh_name, symbol_cnt);
    printf("%7.4s %8.8s %5.4s %-7.7s %-6.6s %-8.8s %3.3s %s\n",
        "Num:", "Value", "Size", "Type", "Bind", "Vis", "Ndx", "Name");

    // print each symbol in symbol table
    static char *symbol_types[STT_NUM] = {"NOTYPE", "OBJECT", "FUNC", "SECTION", "FILE", "COMMON", "TLS"};
    static char *symbol_binds[STB_NUM] = {"LOCAL", "GLOBAL", "WEAK"};
    static char *symbol_vis[] = {"DEFAULT", "INTERNAL", "HIDDEN", "PROTECTED"};

    for (int i = 0; i < symbol_cnt; i++) {
        symbol = symbol_table + i;

        char *index = "";

        unsigned int idx = symbol->st_shndx;
        if (idx >= SHN_LORESERVE && idx <= SHN_HIRESERVE) {
            if (idx == SHN_ABS) {
                index = "ABS";
            } else if (idx == SHN_COMMON) {
                index = "COM";
            } else {
                index = "BAD";
            }
        } else if (idx == SHN_UNDEF) {
            index = "UND";
        } else {
            char buf[3]; // enough
            snprintf(buf, 3, "%d", idx);
            index = buf;
        }

        printf("%6d: %08x %5d %-7s %-6s %-8s %3s %s\n",
                i,
                symbol->st_value,
                symbol->st_size,
                symbol_types[ELF32_ST_TYPE(symbol->st_info)],
                symbol_binds[ELF32_ST_BIND(symbol->st_info)],
                symbol_vis[ELF32_ST_VISIBILITY(symbol->st_other)],
                index,
                str_table + symbol->st_name
        );
    }

    return 0;
}

int display_dynamic_symbols(void) {
    return display_symbols(SHT_DYNSYM);
}

int display_symbol_table(void) {
    return display_symbols(SHT_SYMTAB);
}

int display_dynamic_reloc(void) {
    Elf32_Ehdr *eh = (Elf32_Ehdr *) g_filecnt;
    Elf32_Shdr *sh_base = (Elf32_Shdr *) (g_filecnt + eh->e_shoff);
    Elf32_Shdr *rel_sh = NULL;
    char *dyn_str = NULL;

    // get dynamic relocation section
    for (int i = 0; i < eh->e_shnum; i++) {
        Elf32_Shdr *sh = sh_base + i;
        if (sh->sh_type == SHT_DYNSYM)
            dyn_str = (char *) (g_filecnt + sh_base[sh->sh_link].sh_offset);
        else if (sh->sh_type == SHT_REL)
            rel_sh = sh;
    }

    if (rel_sh == NULL) {
        fprintf(stderr, "readelf: %s: no dynamic relocation section\n", g_filename);
        return 0;
    }

    if (dyn_str == NULL) {
        fprintf(stderr, "readelf: %s: no dynamic string table\n", g_filename);
        return 1;
    }

    // prepare string tables
    Elf32_Shdr *str_sh = sh_base + rel_sh->sh_link;

    static char *reloc_types[] = {
        "R_386_NONE", "R_386_32", "R_386_PC32", "R_386_GOT32", "R_386_PLT32",
        "R_386_COPY", "R_386_GLOB_DAT", "R_386_JMP_SLOT", "R_386_RELATIVE",
        "R_386_GOTOFF", "R_386_GOTPC"
    };

    int size = rel_sh->sh_size / sizeof(Elf32_Rel);

    // print relocation table header
    printf("Offset   Info     Type            Sym Val  Sym Name\n");

    // print each relocation entry in relocation table
    for (int i = 0; i < size; i++) {
        Elf32_Rel *rel = (Elf32_Rel *) (g_filecnt + rel_sh->sh_offset) + i;

        unsigned int type = ELF32_R_TYPE(rel->r_info);
        unsigned int sym  = ELF32_R_SYM(rel->r_info);
        unsigned int sym_value = 0;
        char *sym_name = NULL;

        if (sym != 0) {
            Elf32_Sym *symbol = (Elf32_Sym *) (g_filecnt + str_sh->sh_offset + sym * sizeof(Elf32_Sym));
            sym_value = symbol->st_value;
            sym_name = dyn_str + symbol->st_name;
        }

        printf("%08x %08x %-15s %08x %s\n",
                rel->r_offset, rel->r_info,
                type < sizeof(reloc_types) / sizeof(char *) ? reloc_types[type] : "UNKNOWN",
                sym_value, sym_name ? sym_name : ""
        );
    }

    return 0;
}

int display_needed_libraries(void) {
    Elf32_Ehdr *eh = (Elf32_Ehdr *) g_filecnt;
    Elf32_Shdr *sh_base = (Elf32_Shdr *) (g_filecnt + eh->e_shoff);
    Elf32_Shdr *dyn_sh = NULL;
    char *dyn_str = NULL;

    // get dynamic section
    for (int i = 0; i < eh->e_shnum; i++) {
        Elf32_Shdr *sh = sh_base + i;
        if (sh->sh_type == SHT_DYNAMIC)
            dyn_sh = sh;
        else if (sh->sh_type == SHT_DYNSYM)
            dyn_str = (char *) (g_filecnt + sh_base[sh->sh_link].sh_offset);
    }

    if (dyn_sh == NULL) {
        fprintf(stderr, "readelf: %s: no dynamic section\n", g_filename);
        return 1;
    }

    if (dyn_str == NULL) {
        fprintf(stderr, "readelf: %s: no dynamic string table\n", g_filename);
        return 1;
    }

    for (uint32_t i = 0; i < dyn_sh->sh_size / sizeof(Elf32_Dyn); i++) {
        Elf32_Dyn *dyn = (Elf32_Dyn *) (g_filecnt + dyn_sh->sh_offset) + i;

        if (dyn->d_tag == DT_NEEDED) {
            printf("[NEEDED] %s\n", dyn_str + dyn->d_un.d_val);
        }
    }

    return 0;
}
