/*****************************************************************************\
|   === runtime.c : 2024 ===                                                  |
|                                                                             |
|    Kernel binary execution functions                             .pi0iq.    |
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

int is_valid_elf(void *data) {
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)data;

    if (mem_cmp(&ehdr->e_ident[EI_MAG], ELFMAG, SELFMAG) != 0 ||
        ehdr->e_ident[EI_CLASS] != ELFCLASS32 ||
        ehdr->e_ident[EI_DATA] != ELFDATA2LSB ||
        ehdr->e_type != ET_EXEC ||
        ehdr->e_machine != EM_386
    ) return 0;

    return 1;
}

void *get_base_addr(uint8_t *data) {
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

void *load_sections(uint8_t *file) {
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *) file;
    Elf32_Shdr *shdr = (Elf32_Shdr *)(file + ehdr->e_shoff);

    void *base_addr = get_base_addr(file);
    uint32_t required_size = 0;

    for (int i = 0; i < ehdr->e_shnum; i++) {
        if (shdr[i].sh_addr + shdr[i].sh_size > required_size)
            required_size = shdr[i].sh_addr + shdr[i].sh_size;
    }

    required_size -= (uint32_t) base_addr;
    required_size = ALIGN(required_size, 0x1000);

    scuba_call_generate(base_addr, required_size / 0x1000);
    mem_set(base_addr, 0, required_size);

    for (int i = 0; i < ehdr->e_shnum; i++) {
        if (shdr[i].sh_type == SHT_PROGBITS && shdr[i].sh_addr) {
            mem_copy((void *) shdr[i].sh_addr, file + shdr[i].sh_offset, shdr[i].sh_size);
        }
    }

    return base_addr;
}

int elf_exec(uint32_t sid, char **argv, char **envp) {
    if (!kfu_is_file(sid)) {
        sys_warning("[exec] File not found");
        return -1;
    }

    uint32_t size = fs_cnt_get_size(sid);

    uint8_t *file = mem_alloc(size, SNOW_KERNEL, 0);
    fs_cnt_read(sid, file, 0, size);

    if (size < sizeof(Elf32_Ehdr) || !is_valid_elf(file)) {
        sys_warning("[exec] Not a valid ELF file");
        free(file);
        return -1;
    }

    int pid = process_get_pid();

    int argc = 0;

    if (argv) while (argv[argc] != NULL)
        argc++;

    if (argc)
        process_info(pid, PROC_INFO_SET_NAME, argv[0]);

    scuba_dir_t *old_dir = process_get_dir(pid);
    scuba_dir_t *new_dir = scuba_dir_inited(old_dir, pid);

    // create stack
    void *phys = scuba_create_virtual(new_dir, (void *) PROC_ESP_ADDR, PROC_ESP_SIZE / 0x1000);
    mem_copy(phys, (void *) PROC_ESP_ADDR, PROC_ESP_SIZE);

    // switch to new directory
    process_switch_directory(pid, new_dir, 0);
    scuba_switch(new_dir);
    scuba_dir_destroy(old_dir);

    load_sections(file);
    uint32_t entry = ((Elf32_Ehdr *) file)->e_entry;
    free(file);

    // call the entry point
    sys_exit_kernel(0);
    int ret = ((int (*)(int, char **, char **)) entry)(argc, argv, envp);
    sys_entry_kernel(0);

    return process_kill(process_get_pid(), ret);
}

int elf_start(char *file, int sleep, char **argv, int *pid_ptr) {
    uint32_t sid = kfu_path_to_sid(SID_ROOT, file);

    if (pid_ptr != NULL)
        *pid_ptr = -1;

    if (IS_SID_NULL(sid) || !kfu_is_file(sid)) {
        sys_warning("[elf_start] File not found: %s", file);
        return -1;
    }

    int argc = 0;
    while (argv && argv[argc] != NULL)
        argc++;

    char **nargv = mem_alloc((argc + 1) * sizeof(char *), SNOW_ARGS, 0);
    nargv[argc] = NULL;

    int pid = process_create(elf_exec, 0, 3, (uint32_t []) {sid, (uint32_t) nargv, 0});

    if (pid == -1) {
        free(nargv);
        return -1;
    }

    if (pid_ptr != NULL)
        *pid_ptr = pid;

    mem_alloc_fetch(nargv, pid);

    for (int i = 0; i < argc; i++) {
        nargv[i] = mem_alloc(str_len(argv[i]) + 1, SNOW_ARGS, pid);
        str_cpy(nargv[i], argv[i]);
    }

    if (sleep == 2)
        return 0;

    uint8_t ret;

    process_wakeup(pid, sleep);
    process_wait(pid, &ret, 0);

    return ret;
}
