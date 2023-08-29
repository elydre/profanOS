#include <syscall.h>
#include <string.h>
#include <stdio.h>
#include <type.h>

struct stackframe {
    struct stackframe* ebp;
    uint32_t eip;
};

int main() {
    return 0;
}

void assemble_path(char *old, char *new, char *result) {
    result[0] = '\0';
    if (new[0] == '/') {
        strcpy(result, new);
        return;
    }
    strcpy(result, old);
    if (result[strlen(result) - 1] != '/') {
        strcat(result, "/");
    }
    strcat(result, new);
    if (result[strlen(result) - 1] == '/' && strlen(result) != 1) {
        result[strlen(result) - 1] = '\0';
    }
}

void profan_stacktrace() {
    struct stackframe *stk;
    asm ("movl %%ebp,%0" : "=r"(stk) ::);
    printf("Stack trace:\n");
    int size = 0;
    while (stk->eip) {
        printf("   %x\n", stk->eip);
        stk = stk->ebp;
        size++;
    }
    printf("total size: %d\n", size);
}
