#include <syscall.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define STATUS_ALLOCATED 1
#define STATUS_FREED 0
#define STATUS_NOT_ALLOCATED 2
#define STATUS_DOUBLE_FREE 3

typedef struct memreg_s {
    struct memreg_s *next;
    uint32_t status;

    uint32_t addr;
    uint32_t size;

    const char *file;

    int df_line;
    int line;

    int pid;
} memreg_t;

static memreg_t *memreg_head;

int main(void) {
    memreg_head = NULL;
    return 0;
}

char *kdup(const char *s) {
    char *ret = malloc_ask(strlen(s) + 1);
    strcpy(ret, s);
    return ret;
}

void memreg_alloc(uint32_t addr, uint32_t size, int line, const char *file) {
    memreg_t *reg = malloc_ask(sizeof(memreg_t));

    reg->next = memreg_head;
    memreg_head = reg;

    reg->status = STATUS_ALLOCATED;
    reg->addr = addr;
    reg->size = size;
    reg->file = kdup(file);
    reg->line = line;
    reg->pid = c_process_get_pid();
}

void memreg_never_alloc(uint32_t addr, int line, const char *file) {
    memreg_t *reg = malloc_ask(sizeof(memreg_t));

    reg->next = memreg_head;
    memreg_head = reg;

    reg->status = STATUS_NOT_ALLOCATED;
    reg->addr = addr;
    reg->file = kdup(file);
    reg->line = line;
    reg->pid = c_process_get_pid();
}

void memreg_double_free(uint32_t addr, int line, int df_line, const char *file) {
    memreg_t *reg = malloc_ask(sizeof(memreg_t));

    reg->next = memreg_head;
    memreg_head = reg;

    reg->status = STATUS_DOUBLE_FREE;
    reg->addr = addr;
    reg->file = kdup(file);
    reg->line = line;
    reg->df_line = df_line;
    reg->pid = c_process_get_pid();
}

void memreg_free(uint32_t addr, int line, const char *file) {
    memreg_t *reg = memreg_head;
    memreg_t *other = NULL;
    int pid = c_process_get_pid();

    if (!reg) {
        memreg_never_alloc(addr, line, file);
        return;
    }

    while (reg) {
        if (reg->addr == addr && reg->pid == pid) {
            if (reg->status == STATUS_ALLOCATED) {
                reg->status = STATUS_FREED;
                reg->line = line;
                return;
            } else {
                other = reg;
            }
        }

        reg = reg->next;
    }

    if (other) {
        memreg_double_free(addr, line, other->line, other->file);
    } else {
        memreg_never_alloc(addr, line, file);
    }

}

void memreg_dump(int pid, int color) {
    memreg_t *reg = memreg_head;
    int mallocs = 0;
    int frees = 0;

    if (color) {
        printf("\e[36m");
    }

    int n = printf("\n== Memory dump for process %d ==\n", pid);

    while (reg) {
        if (reg->pid != pid) {
            reg = reg->next;
            continue;
        }
        if (reg->status == STATUS_ALLOCATED) {
            if (color)
                printf(" \e[96mMemory leak\e[36m: %p, %d bytes, allocated at \e[96m%s:%d\e[36m\n", reg->addr, reg->size, reg->file, reg->line);
            else
                printf(" Memory leak: %p, %d bytes, allocated at %s:%d\n", reg->addr, reg->size, reg->file, reg->line);
            mallocs++;
        }
        if (reg->status == STATUS_FREED) {
            mallocs++;
            frees++;
        }
        reg = reg->next;
    }

    reg = memreg_head;

    while (reg) {
        if (reg->status == STATUS_DOUBLE_FREE && reg->pid == pid) {
            if (color)
                printf(" \e[96mDouble free\e[36m: %p, freed at \e[96m%s:%d\e[36m (already freed line %d)\n", reg->addr, reg->file, reg->line, reg->df_line);
            else
                printf(" Double free: %p, freed at %s:%d (already freed line %d)\n", reg->addr, reg->file, reg->line, reg->df_line);
            frees++;
        }
        reg = reg->next;
    }

    reg = memreg_head;

    while (reg) {
        if (reg->status == STATUS_NOT_ALLOCATED && reg->pid == pid) {
            if (color)
                printf(" \e[96mInvalid free\e[36m: %p, freed at \e[96m%s:%d\e[36m\n", reg->addr, reg->file, reg->line);
            else
                printf(" Invalid free: %p, freed at %s:%d\n", reg->addr, reg->file, reg->line);
            frees++;
        }
        reg = reg->next;
    }

    if (color)
        printf(" Summary: \e[96m%d mallocs, %d frees\e[36m\n", mallocs, frees);
    else
        printf(" Summary: %d mallocs, %d frees\n", mallocs, frees);
    for (int i = 0; i < n; i++)
        putchar('=');
    putchar('\n');
    putchar('\n');
    if (color) {
        printf("\e[0m");
    }
}

void memreg_clean(int pid) {
    memreg_t *reg = memreg_head;
    memreg_t *prev = NULL;

    while (reg) {
        if (pid == -1 || reg->pid == pid) {
            if (prev) {
                prev->next = reg->next;
            } else {
                memreg_head = reg->next;
            }
            free((void *) reg->file);
            free(reg);
            reg = prev ? prev->next : memreg_head;
        } else {
            prev = reg;
            reg = reg->next;
        }
    }
}
