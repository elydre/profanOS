#include <syscall.h>
#include <filesys.h>
#include <profan.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int serial_debug(char *frm, ...) {
    va_list args;
    char *str;
    int len;

    va_start(args, frm);
    str = malloc(1024);

    len = vsprintf(str, frm, args);
    c_serial_write(SERIAL_PORT_A, str, len);

    free(str);
    va_end(args);

    return len;
}

void profan_print_memory(void *addr, uint32_t size) {
    for (uint32_t i = 0; i < size / 16 + (size % 16 != 0); i++) {
        printf("%08x: ", (uint32_t) addr + i * 16);

        for (int j = 0; j < 16; j++) {
            if (i * 16 + j < size)
                printf("%02x ", *((unsigned char *) addr + i * 16 + j));
            else
                printf("   ");
            if (j % 4 == 3)
                printf(" ");
        }

        for (int j = 0; j < 16; j++) {
            unsigned char c = *((unsigned char *) addr + i * 16 + j);
            if (i * 16 + j >= size)
                break;
            if (c >= 32 && c <= 126)
                printf("%c", c);
            else
                printf(".");
        }
        printf("\n");
    }
}

char *assemble_path(char *old, char *new) {
    char *result;
    int len;

    if (new[0] == '/') {
        return strdup(new);
    }

    result = malloc(strlen(old) + strlen(new) + 2);
    strcpy(result, old);

    if (result[strlen(result) - 1] != '/') {
        strcat(result, "/");
    }
    strcat(result, new);

    len = strlen(result) - 1;
    if (result[len] == '/' && len > 0) {
        result[len] = '\0';
    }

    return result;
}

int profan_wait_pid(uint32_t pid) {
    uint32_t current_pid = c_process_get_pid();

    if (pid == current_pid || !pid)
        return 0;

    while (c_process_get_state(pid) < 4)
        c_process_sleep(current_pid, 10);

    return c_process_get_info(pid, PROCESS_INFO_EXIT_CODE);
}

int profan_open(char *path, int flags, ...) {
    // mode is ignored, permissions are always 777

    char *cwd = getenv("PWD");
    char *fullpath = cwd ? assemble_path(cwd, path) : strdup(path);

    sid_t sid = fu_path_to_sid(ROOT_SID, fullpath);
    if (IS_NULL_SID(sid) && (flags & O_CREAT)) {
        sid = fu_file_create(0, fullpath);
    }

    if (IS_NULL_SID(sid)) {
        free(fullpath);
        return -1;
    }

    if (flags & O_TRUNC && fu_is_file(sid)) {
        fu_set_file_size(sid, 0);
    }

    int fd = fm_open(fullpath);

    if (fd < 0) {
        free(fullpath);
        return -1;
    }

    if (flags & O_APPEND) {
        fm_lseek(fd, 0, SEEK_END);
    }

    free(fullpath);
    return fd;
}

char *open_input(int *size) {
    char *term = getenv("TERM");
    if (!term)
        return NULL;
    if (strstr(term, "serial"))
        return open_input_serial(size, SERIAL_PORT_A);
    return open_input_keyboard(size, term);
}

// defined in deluge
void profan_cleanup(void) {
    return;
}
