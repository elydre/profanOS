#include <syscall.h>
#include <string.h>
#include <iolib.h>

int main() {
    return 0;
}

int sys_get_setting(char name[]) {
    // read settings from /sys/settings.txt
    // return 0 if not found
    char *settings = c_calloc(c_fs_get_file_size("/sys/settings.txt")*126);
    uint32_t *file = c_fs_declare_read_array("/sys/settings.txt");

    c_fs_read_file("/sys/settings.txt", file);

    for (int i = 0; file[i] != (uint32_t) -1 ; i++)
        settings[i] = (char) file[i];

    c_free(file);

    char line[100];
    char arg[100];
    int line_i = 0;
    int part = 0;
    for (int i = 0; i < str_len(settings); i++) {
        if (part == 0) {
            if (settings[i] == '=') {
                part = 1;
                line[line_i] = '\0';
                line_i = 0;
            } else {
                line[line_i] = settings[i];
                line_i++;
            }
            continue;
        }
        if (settings[i] == '\n') {
            part = 0;
            arg[line_i] = '\0';
            line_i = 0;
            if (str_cmp(line, name))
                continue;
            c_free(settings);
            if (arg[str_len(arg)-1] == '\r')
                arg[str_len(arg)-1] = '\0';
            return ascii_to_int(arg);
        } else {
            arg[line_i] = settings[i];
            line_i++;
        }
    }
    c_free(settings);
    fskprint("Setting %s not found", name);
    return 0;
}
