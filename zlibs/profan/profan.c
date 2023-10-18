#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <syscall.h>
#include <filesys.h>
#include <type.h>

struct stackframe {
    struct stackframe* ebp;
    uint32_t eip;
};

char *kb_map;

int profan_kb_load_map(char *path);

int main() {
    kb_map = NULL;
    if (profan_kb_load_map("/zada/keymap/azerty.map")) {
        printf("Failed to load keyboard map\n");
        return 1;
    }

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

int profan_kb_load_map(char *path) {
    sid_t sid = fu_path_to_sid(ROOT_SID, path);
    if (IS_NULL_SID(sid)) {
        return 1;
    }

    int file_size = fu_get_file_size(sid);
    char *file_content = malloc(file_size + 1);

    fu_file_read(sid, file_content, 0, file_size);
    file_content[file_size] = '\0';


    if (strncmp(file_content, "#KEYMAP", 7)) {
        free(file_content);
        return 1;
    }

    char *tmp = calloc_ask(128, 1);

    int tmp_i = 0;
    for (int i = 7; i < file_size; i++) {
        if (file_content[i] == '\n') {
            continue;
        }
        if (file_content[i] == '\\') {
            i++;
            if (file_content[i] == '\\') {
                tmp[tmp_i++] = '\\';
            } else if (file_content[i] == '0') {
                tmp[tmp_i++] = '\0';
            } else {
                tmp[tmp_i++] = file_content[i];
            }
        } else {
            tmp[tmp_i++] = file_content[i];
        }
        if (tmp_i == 128) {
            break;
        }
    }

    free(file_content);
    free(kb_map);
    kb_map = tmp;
    return 0;
}

char profan_kb_get_char(uint8_t scancode, uint8_t shift) {
    if (scancode > 64)
        return '\0';
    if (kb_map == NULL)
        return c_kb_scancode_to_char(scancode, shift);
    if (shift)
        return kb_map[scancode * 2 + 1];
    return kb_map[scancode * 2];
}
