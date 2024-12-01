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
#include <minilib.h>
#include <system.h>

int is_valid_elf(void *data) {
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)data;
    return !(
        memcmp(ehdr->e_ident, (void *) ELFMAG, SELFMAG) != 0 ||
        ehdr->e_type != ET_EXEC || ehdr->e_machine != EM_386
    );
}

int elf_exec(uint32_t sid, int argc, char **argv, char **envp) {
    if (!fu_is_file(fs_get_main(), sid)) {
        sys_warning("[exec] File not found");
        return -1;
    }

    uint32_t size = fu_file_get_size(sid);

    uint8_t *file = mem_alloc(size, 0, 6);
    fu_file_read(sid, file, 0, size);

    if (obj->size < sizeof(Elf32_Ehdr) || !is_valid_elf(obj->file)) {
        sys_warning("[exec] Not a valid ELF file");
        return -1;
    }

    Elf32_Ehdr *ehdr = (Elf32_Ehdr *) file;
    Elf32_Shdr *shdr = (Elf32_Shdr *) (file + ehdr->e_shoff);

    Elf32_Phdr *phdr = (Elf32_Phdr *) (file + ehdr->e_phoff);





}

int run_ifexist(char *file, int sleep, char **argv, int *pid_ptr) {
    uint32_t sid = fu_path_to_sid(fs_get_main(), SID_ROOT, file);

    if (IS_SID_NULL(sid) || !fu_is_file(fs_get_main(), sid)) {
        sys_warning("[run_ifexist] File not found: %s", file);
        return -1;
    }

    int argc = 0;
    while (argv && argv[argc] != NULL)
        argc++;

    char **nargv = mem_alloc((argc + 1) * sizeof(char *), 0, 6);
    mem_set((void *) nargv, 0, (argc + 1) * sizeof(char *));

    for (int i = 0; i < argc; i++) {
        nargv[i] = mem_alloc(str_len(argv[i]) + 1, 0, 6);
        str_cpy(nargv[i], argv[i]);
    }

    int pid = process_create(elf_exec, 0, 4, (uint32_t []) {sid, 0, (uint32_t) nargv, 0});

    if (pid_ptr != NULL)
        *pid_ptr = pid;

    if (pid == -1)
        return -1;

    if (sleep == 2)
        return 0;

    uint8_t ret;

    process_wakeup(pid, sleep);
    process_wait(pid, &ret, 0);

    return ret;
}
