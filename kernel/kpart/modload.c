/*****************************************************************************\
|   === modload.c : 2025 ===                                                  |
|                                                                             |
|    Kernel Dynamic Module loader                                  .pi0iq.    |
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

#define MODLOAD_FARRAY_MAGIC 0xF3A3C4D4
#define MODLOAD_FARRAY_NAME  "__module_func_array"

uint32_t *g_mod_funcs[256];

/* g_mod_funcs[n] layout:
 * 0: binary memory
 * 1: function count
 * 2: constructor (or NULL)
 * 3: destructor  (or NULL)
 * 4: function 1
 * 5: function 2
 * ...
 */

#define IS_LOADED(id) ((id) && (id) < 256 && g_mod_funcs[id])

static uint8_t *i_mod_read_file(uint32_t file_sid) {
    uint32_t file_size = fs_cnt_get_size(file_sid);
    uint8_t *file = malloc(file_size);

    fs_cnt_read(file_sid, file, 0, file_size);

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

static uint8_t *i_mod_resolve(uint8_t *file) {
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *) file;
    Elf32_Shdr *shdr = (Elf32_Shdr *)(file + ehdr->e_shoff);

    uint32_t required_size = 0;

    for (int i = 0; i < ehdr->e_shnum; i++) {
        if (shdr[i].sh_addr + shdr[i].sh_size > required_size)
            required_size = shdr[i].sh_addr + shdr[i].sh_size;
    }

    required_size = (required_size + 0xFFF) & ~0xFFF;

    uint8_t *mem = mem_alloc(required_size, SNOW_MOD, 0x1000);
    mem_set(mem, 0, required_size);

    for (int i = 0; i < ehdr->e_shnum; i++) {
        if (shdr[i].sh_type == SHT_PROGBITS && shdr[i].sh_addr)
            mem_copy(mem + shdr[i].sh_addr, file + shdr[i].sh_offset, shdr[i].sh_size);
    }

    return mem;
}

static uint32_t *i_mod_read_funcs(uint8_t *file, uint8_t *mem) {
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

    if (sym_sh == NULL)
        return NULL;

    // read a first time to get function count
    Elf32_Sym *symbol_table = (Elf32_Sym *) (file + sym_sh->sh_offset);
    char *strtab = (char *) (file + shdr[sym_sh->sh_link].sh_offset);

    uint32_t *func_array = NULL;
    uint32_t func_count = 0;

    // search for the function array
    for (uint32_t i = 0; i < sym_sh->sh_size / sizeof(Elf32_Sym); i++) {
        Elf32_Sym *symbol = symbol_table + i;

        if (symbol->st_name && str_cmp(strtab + symbol->st_name, MODLOAD_FARRAY_NAME) == 0) {
            func_array = (uint32_t *) (mem + symbol->st_value);
            if (func_array[0] != MODLOAD_FARRAY_MAGIC)
                return NULL;
            func_count = symbol->st_size / sizeof(uint32_t) - 1;
            break;
        }
    }

    if (func_array == NULL)
        return NULL;

    // allocate the address list
    uint32_t *addr_list = mem_alloc((func_count + 4) * sizeof(uint32_t), SNOW_MOD, 0);
    addr_list[0] = (uint32_t) mem;
    addr_list[1] = func_count;
    addr_list[2] = 0;   // __init address
    addr_list[3] = 0;   // __fini address

    for (uint32_t i = 0; i < sym_sh->sh_size / sizeof(Elf32_Sym); i++) {
        Elf32_Sym *symbol = symbol_table + i;

        if (symbol->st_info != 0x12 || !symbol->st_name)
            continue;

        else if (str_cmp(strtab + symbol->st_name, "__init") == 0)
            addr_list[2] = symbol->st_value + (uint32_t) mem;

        else if (str_cmp(strtab + symbol->st_name, "__fini") == 0)
            addr_list[3] = symbol->st_value + (uint32_t) mem;
    }

    // copy function addresses
    for (uint32_t i = 0; i < func_count; i++) {
        addr_list[i + 4] = func_array[i + 1];
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

static int i_mod_relocate(char *finename, uint8_t *file, uint8_t *mem) {
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)file;
    Elf32_Shdr *shdr = (Elf32_Shdr *)(file + ehdr->e_shoff);

    uint32_t val = 0;
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
            type = ELF32_R_TYPE(rel[j].r_info);

            if (does_type_required_sym(type)) {
                Elf32_Sym *sym = dynsym + ELF32_R_SYM(rel[j].r_info);
                if ((val = sym->st_value)) {
                    val += (uint32_t) mem;
                } else if (!(val = sys_name2addr(dynstr + sym->st_name))) {
                    sys_warning("[pok load] %s requires symbol '%s'\n", finename, dynstr + sym->st_name);
                    return 1;
                }
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

int mod_load(char *path, uint32_t lib_id) {
    if (path == NULL)
        return IS_LOADED(lib_id) ? (int) g_mod_funcs[lib_id][1] : -1;

    uint32_t file = kfu_path_to_sid(SID_ROOT, path);

    if (SID_IS_NULL(file) || !kfu_is_file(file)) {
        return -1;
    }

    if (!lib_id || lib_id >= 256) {
        return -2;
    }

    if (IS_LOADED(lib_id)) {
        mod_unload(lib_id);
    }

    uint8_t *elf = i_mod_read_file(file);

    if (elf == NULL)
        return -3;

    uint8_t *binary_mem = i_mod_resolve(elf);

    if (i_mod_relocate(path, elf, binary_mem)) {
        free(binary_mem);
        free(elf);
        return -4;
    }

    uint32_t *addr_list = i_mod_read_funcs(elf, binary_mem);
    free(elf);

    if (addr_list == NULL) {
        free(binary_mem);
        return -5;
    }

    int ret_val;

    if (addr_list[2] && (ret_val = ((int (*)(void)) addr_list[2])())) {
        free(binary_mem);
        free(addr_list);
        return -(6 + (ret_val == 2));
    }

    // save the address list
    g_mod_funcs[lib_id] = addr_list;

    // return the function count
    return addr_list[1];
}

int mod_unload(uint32_t lib_id) {
    if (!IS_LOADED(lib_id)) {
        sys_warning("Module %d not loaded", lib_id);
        return 1;
    }

    if (g_mod_funcs[lib_id][3]) {
        ((void (*)(void)) g_mod_funcs[lib_id][3])();
    }

    free((void *) g_mod_funcs[lib_id][0]);
    free(g_mod_funcs[lib_id]);

    g_mod_funcs[lib_id] = NULL;
    return 0;
}

uint32_t mod_get_func(uint32_t lib_id, uint32_t func_id) {
    if (!IN_KERNEL) {
        sys_entry_kernel();
        sys_error("mod_get_func called from userspace");
        return 0;
    }

    if (!IS_LOADED(lib_id)) {
        sys_error("Module %d (func %d) not loaded requested by pid %d (addr)", lib_id, func_id, process_get_pid());
        return 0;
    }

    uint32_t *addr_list = g_mod_funcs[lib_id];

    if (func_id >= addr_list[1]) {
        sys_error("Function %d not found in library %d", func_id, lib_id);
        return 0;
    }

    return addr_list[func_id + 4];
}

void mod_syscall(registers_t *r) {
    uint32_t lib_id = r->eax >> 24;
    uint32_t func_id = r->eax & 0xFFFFFF;

    if (!IS_LOADED(lib_id)) {
        sys_error("Module %d (func %d) not loaded requested by pid %d (call)", lib_id, func_id, process_get_pid());
        return;
    }

    uint32_t *addr_list = g_mod_funcs[lib_id];

    if (func_id >= addr_list[1]) {
        sys_error("Function %d not found in library %d", func_id, lib_id);
        return;
    }

    uint32_t (*func)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t) = (void *) addr_list[func_id + 4];
    r->eax = func(r->ebx, r->ecx, r->edx, r->esi, r->edi);
}
