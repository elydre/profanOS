/*****************************************************************************\
|   === pok.c : 2024 ===                                                      |
|                                                                             |
|    Kernel Dynamic Library loader                                 .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <kernel/snowflake.h>
#include <kernel/butterfly.h>
#include <kernel/process.h>
#include <kernel/tinyelf.h>
#include <minilib.h>
#include <system.h>

uint32_t **lib_functions = 0;

int pok_init(void) {
    *(int *)(WATPOK_ADDR) = (int) pok_get_func;
    return 0;
}

static int i_pok_does_loaded(uint32_t lib_id) {
    return !(
        (lib_id < 1000            ||
         lib_id >= POK_MAX + 1000 ||
         lib_functions == 0
        ) || (
         lib_functions[lib_id - 1000] == 0
        )
    );
}

static uint8_t *i_pok_read_file(uint32_t file_sid) {
    uint32_t file_size = fs_cnt_get_size(fs_get_main(), file_sid);
    uint8_t *file = malloc(file_size);

    fs_cnt_read(fs_get_main(), file_sid, file, 0, file_size);

    // check if the file is an ELF 32 dynamic library
    if (mem_cmp(file, ELFMAG, SELFMAG) != 0) {
        free(file);
        return NULL;
    }

    Elf32_Ehdr *ehdr = (Elf32_Ehdr *) file;
    if (ehdr->e_type != ET_DYN || ehdr->e_machine != EM_386) {
        free(file);
        return NULL;
    }

    return file;
}

static uint8_t *i_pok_resolve(uint8_t *file) {
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *) file;
    Elf32_Shdr *shdr = (Elf32_Shdr *)(file + ehdr->e_shoff);

    uint32_t required_size = 0;

    for (int i = 0; i < ehdr->e_shnum; i++) {
        if (shdr[i].sh_addr + shdr[i].sh_size > required_size)
            required_size = shdr[i].sh_addr + shdr[i].sh_size;
    }

    required_size = (required_size + 0xFFF) & ~0xFFF;

    uint8_t *mem = mem_alloc(required_size, 0x1000, 5); // 5: library
    mem_set(mem, 0, required_size);

    for (int i = 0; i < ehdr->e_shnum; i++) {
        if (shdr[i].sh_type == SHT_PROGBITS && shdr[i].sh_addr) {
            mem_copy(mem + shdr[i].sh_addr, file + shdr[i].sh_offset, shdr[i].sh_size);
        }
    }

    return mem;
}

static uint32_t *i_pok_read_funcs(uint8_t *file, uint8_t *mem) {
    // read the symbol table and save function addresses in an array

    Elf32_Ehdr *ehdr = (Elf32_Ehdr *) file;
    Elf32_Shdr *shdr = (Elf32_Shdr *)(file + ehdr->e_shoff);

    Elf32_Shdr *sym_sh = NULL;

    for (int i = 0; i < ehdr->e_shnum; i++) {
        if (shdr[i].sh_type == SHT_SYMTAB) {
            sym_sh = shdr + i;
            break;
        }
    }

    if (sym_sh == NULL) {
        return NULL;
    }

    // read a first time to get function count
    uint32_t func_count = 0;
    Elf32_Sym *symbol_table = (Elf32_Sym *) (file + sym_sh->sh_offset);

    for (uint32_t i = 0; i < sym_sh->sh_size / sizeof(Elf32_Sym); i++) {
        if (((Elf32_Sym *) symbol_table + i)->st_info == 0x12) {
            func_count++;
        }
    }

    if (func_count == 0) {
        return NULL;
    }

    // allocate the address list
    uint32_t *addr_list = mem_alloc((func_count + 2) * sizeof(uint32_t), 0, 5); // 5: library
    addr_list[0] = (uint32_t) mem;
    addr_list[1] = func_count;
    func_count = 2;

    for (uint32_t i = 0; i < sym_sh->sh_size / sizeof(Elf32_Sym); i++) {
        Elf32_Sym *symbol = symbol_table + i;

        if (symbol->st_info == 0x12) {
            addr_list[func_count++] = symbol->st_value + (uint32_t) mem;
        }
    }

    // sort the functions by address
    for (uint32_t i = 2; i < func_count; i++) {
        for (uint32_t j = i + 1; j < func_count; j++) {
            if (addr_list[i] > addr_list[j]) {
                uint32_t tmp = addr_list[i];
                addr_list[i] = addr_list[j];
                addr_list[j] = tmp;
            }
        }
    }

    return addr_list;
}

static int does_type_required_sym(uint8_t type) {
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

static int i_pok_relocate(char *finename, uint8_t *file, uint8_t *mem) {
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)file;
    Elf32_Shdr *shdr = (Elf32_Shdr *)(file + ehdr->e_shoff);

    uint32_t val;
    uint8_t type;

    char *dynstr = NULL;
    Elf32_Sym *dynsym = NULL;

    for (int i = 0; i < ehdr->e_shnum; i++) {
        if (shdr[i].sh_type == 11) { // SHT_DYNSYM
            dynsym = (Elf32_Sym *)(file + shdr[i].sh_offset);
            dynstr = (char *) file + shdr[shdr[i].sh_link].sh_offset;
            break;
        }
    }

    if (dynsym == NULL || dynstr == NULL) {
        return 1;
    }

    for (uint32_t i = 0; i < ehdr->e_shnum; i++) {
        if (shdr[i].sh_type == 4) // SHT_RELA
            return 1;

        if (shdr[i].sh_type != 9) // SHT_REL
            continue;

        Elf32_Rel *rel = (Elf32_Rel *)(file + shdr[i].sh_offset);
        for (uint32_t j = 0; j < shdr[i].sh_size / sizeof(Elf32_Rel); j++) {
            val = 0;
            type = ELF32_R_TYPE(rel[j].r_info);
            if (does_type_required_sym(type)) {
                Elf32_Sym *sym = dynsym + ELF32_R_SYM(rel[j].r_info);
                val = sym->st_value;
                if (!val) {
                    sys_warning("[pok load] symbol %s not found in %s", dynstr + sym->st_name, finename);
                    return 1;
                }
                val += (uint32_t) mem;
            }
            switch (type) {
                case R_386_32:          // word32  S + A
                    val += *(uint32_t *)(mem + rel[j].r_offset);
                    *(uint32_t *)(mem + rel[j].r_offset) = val;
                    break;
                case R_386_PC32:        // word32  S + A - P
                    val += *(uint32_t *)(mem + rel[j].r_offset);
                    val -= (uint32_t) (mem + rel[j].r_offset);
                    *(uint32_t *)(mem + rel[j].r_offset) = val;
                    break;
                case R_386_RELATIVE:    // word32  B + A
                    val = (uint32_t) mem;
                    val += *(uint32_t *)(mem + rel[j].r_offset);
                    *(uint32_t *)(mem + rel[j].r_offset) = val;
                    break;
                case R_386_JMP_SLOT:    // word32  S
                    *(uint32_t *)(mem + rel[j].r_offset) = val;
                    break;
                case R_386_GLOB_DAT:    // word32  S
                    *(uint32_t *)(mem + rel[j].r_offset) = val;
                    break;
                default:
                    sys_warning("[pok load] relocation type %d in %s\n", type, finename);
                    return 1;
            }
        }
    }
    return 0;
}

int pok_load(char *path, uint32_t lib_id) {
    uint32_t file = fu_path_to_sid(fs_get_main(), SID_ROOT, path);
    if (IS_SID_NULL(file) || !fu_is_file(fs_get_main(), file)) {
        return -1;
    }

    if (lib_id < 1000 || lib_id >= POK_MAX + 1000) {
        return -2;
    }

    if (i_pok_does_loaded(lib_id)) {
        pok_unload(lib_id);
    }

    // allocate the lib_functions array if it doesnt exist
    if (lib_functions == 0) {
        lib_functions = mem_alloc(POK_MAX * sizeof(uint32_t *), 0, 5);    // 5: library
        mem_set((uint8_t *) lib_functions, 0, POK_MAX * sizeof(uint32_t *));
    }

    uint8_t *elf = i_pok_read_file(file);

    if (elf == NULL)
        return -3;

    uint8_t *binary_mem = i_pok_resolve(elf);

    if (i_pok_relocate(path, elf, binary_mem)) {
        free(binary_mem);
        free(elf);
        return -4;
    }

    uint32_t *addr_list = i_pok_read_funcs(elf, binary_mem);
    free(elf);

    if (addr_list == NULL) {
        free(binary_mem);
        return -5;
    }

    sys_exit_kernel(0);
    int ret_code = ((int (*)(void)) addr_list[2])();
    sys_entry_kernel(0);

    if (ret_code) {
        free(binary_mem);
        free(addr_list);
        return -6;
    }

    // save the address list
    lib_functions[lib_id - 1000] = addr_list;

    // return the function count
    return addr_list[1];
}

int pok_unload(uint32_t lib_id) {
    if (!i_pok_does_loaded(lib_id)) {
        sys_warning("Library %d not loaded", lib_id);
        return 1;
    }
    free((void *) lib_functions[lib_id - 1000][0]);
    free(lib_functions[lib_id - 1000]);
    lib_functions[lib_id - 1000] = 0;

    return 0;
}

uint32_t pok_get_func(uint32_t lib_id, uint32_t func_id) {
    if (!i_pok_does_loaded(lib_id)) {
        sys_error("Library %d (func %d) not loaded requested by pid %d", lib_id, func_id, process_get_pid());
        return 0;
    }

    uint32_t *addr_list = lib_functions[lib_id - 1000];
    if (func_id > addr_list[1]) {
        sys_error("Function %d not found in library %d", func_id, lib_id);
        return 0;
    }

    return addr_list[func_id + 1];
}
