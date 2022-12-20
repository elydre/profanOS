#include <syscall.h>
#include <type.h>
#include <i_string.h>
#include <i_iolib.h>
#include <i_mem.h>

int main() {
    return 0;
}

int setting_get(char name[]) {
    // read settings from /sys/settings.txt
    // return 0 if not found
    char *settings = c_fs_declare_read_array("/sys/settings.txt");

    c_fs_read_file("/sys/settings.txt", (uint8_t *) settings);

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
            free(settings);
            if (arg[str_len(arg)-1] == '\r')
                arg[str_len(arg)-1] = '\0';
            return ascii_to_int(arg);
        } else {
            arg[line_i] = settings[i];
            line_i++;
        }
    }
    free(settings);
    fsprint("Setting %s not found", name);
    return 0;
}
