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

void assemble_path(char old[], char new[], char result[]) {
    result[0] = '\0';
    if (new[0] != '/') {
        strcpy(result, old);
    }
    if (result[strlen(result) - 1] != '/') {
        strncat(result, "/", 1);
    }
    int index;
    for (unsigned int i = 0; i < strlen(new); i++) {
        if (new[i] == '.' && new[i + 1] == '.' && (new[i + 2] == '/' || new[i + 2] == '\0')) {
            for (int j = strlen(result) - 2; j >= 0; j--) {
                if (result[j] == '/') {
                    result[j + 1] = '\0';
                    break;
                }
            }
            i += 2;
        } else if (new[i] == '.' && new[i + 1] == '/') {
            i++;
        } else if (new[i] == '/' && result[strlen(result) - 1] == '/') {
            continue;
        } else {
            index = strlen(result);
            result[index] = new[i];
            result[index + 1] = '\0';
        }
    }
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
