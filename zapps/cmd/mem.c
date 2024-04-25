/****** This file is part of profanOS **************************\
|   == mem.c ==                                      .pi0iq.    |
|                                                   d"  . `'b   |
|   Command to display memory usage                 q. /|\  u   |
|                                                    `// \\     |
|                                                    //   \\    |
|   [ github.com/elydre/profanOS - GPLv3 ]          //     \\   |
\***************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <profan/syscall.h>
#include <profan.h>

#define PROCESS_MAX 20

int str_to_int(char *str) {
    int ret = 0;
    if (str[0] != '0' || str[1] != 'x') {
        for (int i = 0; str[i]; i++) {
            if (str[i] >= '0' && str[i] <= '9')
                ret = ret * 10 + str[i] - '0';
            else if (str[i] != ' ')
                return -1;
        }
        return ret;
    }
    for (int i = 2; str[i]; i++) {
        if (str[i] >= '0' && str[i] <= '9')
            ret = ret * 16 + str[i] - '0';
        else if (str[i] >= 'a' && str[i] <= 'f')
            ret = ret * 16 + str[i] - 'a' + 10;
        else if (str[i] >= 'A' && str[i] <= 'F')
            ret = ret * 16 + str[i] - 'A' + 10;
        else if (str[i] != ' ')
            return -1;
    }
    return ret;
}

int print_help(int full) {
    puts(
        "Usage: mem [options]\n"
        "Options:\n"
        "  -p <f> [s] print memory area\n"
        "  -h         display this help message\n"
        "  -l         show detailed memory usage\n"
        "  -s         show summary of memory usage"
    );

    return 0;
}

typedef struct {
    uint32_t start;
    uint32_t size;
    int mode;
} mem_args_t;

#define ACTION_PRINT 1
#define ACTION_LIST  2
#define ACTION_SUM   3
#define ACTION_HELP  4
#define ACTION_ERROR 5

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
    } else if (argc == 3) {
        if (strcmp(argv[1], "-p") == 0) {
            ret->mode = ACTION_PRINT;
            ret->start = str_to_int(argv[2]);
            ret->size = 512;
        }
    } else if (argc == 4) {
        if (strcmp(argv[1], "-p") == 0) {
            ret->mode = ACTION_PRINT;
            ret->start = str_to_int(argv[2]);
            ret->size = str_to_int(argv[3]);
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
    int alloc_count = c_mem_get_info(4, 0);
    int free_count = c_mem_get_info(5, 0);

    printf("      -------- General --------\n");

    printf("Total memory      %12d MB\n", c_mem_get_info(0, 0) / 1024 / 1024);
    printf("AT alloc count    %15d\n", alloc_count);
    printf("AT free count     %15d\n", free_count);
    printf("base address      %15p\n", c_mem_get_info(1, 0));
    printf("MEM_PARTS addr    %15p\n", c_mem_get_info(3, 0));
    printf("MEM_PARTS size    %12d B  | 1\n", c_mem_get_info(2, 0));
    printf("Used memory       %12d kB | %d\n", c_mem_get_info(6, 0) / 1024, alloc_count - free_count);

    printf("\n      ------- Per types -------\n");

    printf("Simple alloc   %15d kB | %d\n", c_mem_get_info(10, 1) / 1024, c_mem_get_info(9, 1));
    printf("Mem struct     %15d kB | %d\n", c_mem_get_info(10, 3) / 1024, c_mem_get_info(9, 3));
    printf("Process stack  %15d kB | %d\n", c_mem_get_info(10, 4) / 1024, c_mem_get_info(9, 4));
    printf("Dily           %15d kB | %d\n", c_mem_get_info(10, 5) / 1024, c_mem_get_info(9, 5));
    printf("As kernel      %15d kB | %d\n", c_mem_get_info(10, 6) / 1024, c_mem_get_info(9, 6));
    printf("Scuba vpage    %15d kB | %d\n", c_mem_get_info(10, 7) / 1024, c_mem_get_info(9, 7));

    printf("\n      ------ Per process ------\n");

    uint32_t pid_list[PROCESS_MAX]; // it's a define
    int pid_list_len = c_process_generate_pid_list(pid_list, PROCESS_MAX);
    sort_tab(pid_list, pid_list_len);

    int pid;
    char *name;
    for (int i = 0; i < pid_list_len; i++) {
        pid = pid_list[i];
        if (pid == 1) continue;
        name = (char *) c_process_get_info(pid, PROCESS_INFO_NAME);
        printf("PID %-3.02d %-15s %6d kB | %d\n",
                pid, (strchr(name, '/') ? strrchr(name, '/') + 1 : name),
                c_mem_get_info(8, pid) / 1024,
                c_mem_get_info(7, pid)
        );
    }
    printf("\n");
}

void memory_print_summary(void) {
    int current = c_mem_get_info(6, 0);
    int total = c_mem_get_info(0, 0);
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
        print_help(1);
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

    if (args->mode == ACTION_PRINT) {
        profan_print_memory((void *) args->start, args->size);
        free(args);
        return 0;
    }

    fputs("mem: Internal error\n", stderr);

    free(args);
    return 1;
}
