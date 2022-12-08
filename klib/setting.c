#include <klib/filesystem.h>
#include <string.h>
#include <type.h>
#include <mem.h>

int sys_get_setting(char name[]) {
    // read settings from /sys/settings.txt
    // return 0 if not found
    char *settings = calloc(fs_get_file_size("/sys/settings.txt")*126);
    uint32_t *file = fs_declare_read_array("/sys/settings.txt");

    fs_read_file("/sys/settings.txt", file);

    for (int i = 0; file[i] != (uint32_t) -1 ; i++)
        settings[i] = (char) file[i];

    free(file);

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
    sys_warning("Setting not found");
    return 0;
}
