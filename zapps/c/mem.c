/*****************************************************************************\
|   === mem.c : 2024 ===                                                      |
|                                                                             |
|    Command to display memory usage                               .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <profan/syscall.h>
#include <profan.h>

#define PROCESS_MAX 64

int print_help(void) {
    puts(
        "Usage: mem [options]\n"
        "Options:\n"
        "  -h     display this help message\n"
        "  -l     show detailed memory usage\n"
        "  -s     show summary of memory usage"
    );

    return 0;
}

typedef struct {
    uint32_t start;
    uint32_t size;
    int mode;
} mem_args_t;

#define ACTION_LIST  1
#define ACTION_SUM   2
#define ACTION_HELP  3
#define ACTION_ERROR 4

mem_args_t *parse_args(int argc, char **argv) {
    mem_args_t *ret = malloc(sizeof(mem_args_t));
    ret->mode = ACTION_ERROR;

    if (argc == 1) {
        ret->mode = ACTION_LIST;
    } else if (argc == 2) {
        if (strcmp(argv[1], "-h") == 0) {
            ret->mode = ACTION_HELP;
        } else if (strcmp(argv[1], "-l") == 0) {
            ret->mode = ACTION_LIST;
        } else if (strcmp(argv[1], "-s") == 0) {
            ret->mode = ACTION_SUM;
        }
    }

    return ret;
}

void sort_tab(uint32_t *tab, int size) {
    int tmp;
    for (int i = 0; i < size; i++) {
        for (int j = i; j > 0 && tab[j] < tab[j - 1]; j--) {
            tmp = tab[j];
            tab[j] = tab[j - 1];
            tab[j - 1] = tmp;
        }
    }
}

void memory_print_usage(void) {
    int alloc_count = syscall_mem_info(4, 0);
    int free_count = syscall_mem_info(5, 0);

    printf("      -------- General --------\n");

    printf("Total memory      %12d MB\n", syscall_mem_info(0, 0) / 1024 / 1024);
    printf("AT alloc count    %15d\n", alloc_count);
    printf("AT free count     %15d\n", free_count);
    printf("base address      %15p\n", syscall_mem_info(1, 0));
    printf("MEM_PARTS addr    %15p\n", syscall_mem_info(3, 0));
    printf("MEM_PARTS size    %12d B  | 1\n", syscall_mem_info(2, 0));
    printf("Used memory       %12d kB | %d\n", syscall_mem_info(6, 0) / 1024, alloc_count - free_count);

    printf("\n      ------- Per types -------\n");

    printf("Simple alloc   %15d kB | %d\n", syscall_mem_info(10, 1) / 1024, syscall_mem_info(9, 1));
    printf("Mem struct     %15d kB | %d\n", syscall_mem_info(10, 3) / 1024, syscall_mem_info(9, 3));
    printf("Scuba vpage    %15d kB | %d\n", syscall_mem_info(10, 4) / 1024, syscall_mem_info(9, 4));
    printf("pok            %15d kB | %d\n", syscall_mem_info(10, 5) / 1024, syscall_mem_info(9, 5));
    printf("As kernel      %15d kB | %d\n", syscall_mem_info(10, 6) / 1024, syscall_mem_info(9, 6));

    printf("\n      ------ Per process ------\n");

    uint32_t pid_list[PROCESS_MAX]; // it's a define
    int pid_list_len = syscall_process_list_all(pid_list, PROCESS_MAX);
    sort_tab(pid_list, pid_list_len);

    int pid;
    char *name;
    for (int i = 0; i < pid_list_len; i++) {
        pid = pid_list[i];
        if (pid == 1) continue;
        name = (char *) syscall_process_info(pid, PROC_INFO_NAME, NULL);
        printf("PID %-3.02d %-15s %6d kB | %-3d (%d)\n",
                pid, (strchr(name, '/') ? strrchr(name, '/') + 1 : name),
                syscall_mem_info(12, pid) / 1024,
                syscall_mem_info(11, pid),
                syscall_mem_info(7, pid)
        );
    }
    printf("\n");
}

void memory_print_summary(void) {
    int current = syscall_mem_info(6, 0);
    int total = syscall_mem_info(0, 0);
    printf("About %.2f %% of memory is currently used,\n", (float) current / (float) total * 100);
    printf("In total, %d MB can be allocated!\n", total / 1024 / 1024);
}

int main(int argc, char *argv[]) {
    mem_args_t *args = parse_args(argc, argv);

    if (args->mode == ACTION_ERROR) {
        fputs("mem: Invalid argument\n", stderr);
        fputs("Try 'mem -h' for more information.\n", stderr);
        free(args);
        return 1;
    }

    if (args->mode == ACTION_HELP) {
        print_help();
        free(args);
        return 0;
    }

    if (args->mode == ACTION_LIST) {
        memory_print_usage();
        free(args);
        return 0;
    }

    if (args->mode == ACTION_SUM) {
        memory_print_summary();
        free(args);
        return 0;
    }

    fputs("mem: Internal error\n", stderr);

    free(args);
    return 1;
}
