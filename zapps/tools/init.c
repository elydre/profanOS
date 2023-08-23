#include <syscall.h>
#include <filesys.h>
#include <i_iolib.h>
#include <panda.h>

#include <stdlib.h>
#include <stdio.h>

typedef struct {
    int id;
    char path[256];
} lib_t;

lib_t libs_at_boot[] = {
    {1000, "/lib/profan/iolib.bin"},
    {1002, "/lib/profan/profan.bin"},
    {1004, "/lib/profan/itime.bin"},
    {1006, "/lib/profan/vgui.bin"},
    {1007, "/lib/ports/stdlib.bin"},
    {1008, "/lib/ports/string.bin"},
    {1010, "/lib/profan/filesys.bin"},
    {1015, "/lib/profan/devio.bin"},
    {1009, "/lib/ports/stdio.bin"},
    {1011, "/lib/ports/math.bin"},
    {1012, "/lib/ports/time.bin"},
    {1013, "/lib/ports/setjmp.bin"},
    {1014, "/lib/ports/unistd.bin"},
    {1005, "/lib/profan/panda.bin"},
};

int local_strlen(char *str) {
    int i = 0;
    while (str[i] != 0) {
        i++;
    }
    return i;
}

char *get_name(char *path) {
    int len = local_strlen(path);
    int i = len - 1;
    while (i >= 0 && path[i] != '/') {
        i--;
    }
    return path + i + 1;
}

int print_load_status(int i) {
    lib_t *lib = &libs_at_boot[i];
    if (c_dily_load(lib->path, lib->id)) {
        c_kprint("FAILED TO LOAD ");
        c_kprint(get_name(lib->path));
        c_kprint("LIBRARY\n");
        return 1;
    }
    return 0;
}

int redirect_stdout(char *file) {
    sid_t sid = fu_path_to_sid(ROOT_SID, file);

    return (IS_NULL_SID(sid)
        || !(fu_is_file(sid) || fu_is_fctf(sid))
        || devio_change_redirection(DEVIO_STDOUT, sid)
    );
}

void welcome_print() {
    rainbow_print("Welcome to profanOS!\n");
    char *kver = malloc(256);
    c_sys_kinfo(kver);

    printf("$CKernel: $4%s$$\n\n", kver);
    free(kver);
}

int main() {
    int loaded = 0;
    int total = (int) (sizeof(libs_at_boot) / sizeof(lib_t));
    for (int i = 0; i < total; i++) {
        loaded += !print_load_status(i);
    }
    printf("Loaded %d/%d libraries\n\n", loaded, total);

    panda_set_start(c_get_cursor_offset());

    if (redirect_stdout("/dev/panda")) {
        c_kprint("Failed to redirect stdout\n");
        return 1;
    }
    welcome_print();
    c_run_ifexist("/bin/fatpath/olivine.bin", 0, NULL);

    return 0;
}
