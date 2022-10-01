// requires /user/star_wars.txt, the ascii art of the star wars
// be careful, the ascii movie is more than 2048 sectors long

#include <stdint.h>
#include "addf.h"


int main(int addr, int arg) {
    INIT_AF(addr);

    AF_fs_declare_read_array();
    AF_fs_read_file();
    AF_ascii_to_int();
    AF_clear_screen();
    AF_cursor_blink();
    AF_ckprint_at();
    AF_ms_sleep();
    AF_fskprint();
    AF_malloc();

    char path[] = "/user/star_wars.txt";


    uint32_t * data = fs_declare_read_array(path);
    char * str = malloc(0x4000);

    str[0] = '\0';

    fskprint("loading file: %s into memory...\n", path);
    fs_read_file(path, data);

    cursor_blink(1);
    clear_screen();

    int line = 0, str_index = 0, j;

    char temps[5];

    for (int i = 0; 1; i++) {
        str_index++;
        str[str_index] = (char) data[i];
        if (str[str_index] != '\n') continue;
        line++;
        if (line % 14) continue;
        for (j = 0; str[j] != '\n'; j++) temps[j] = str[j];
        temps[j] = '\0';
        str[str_index] = '\0';
        clear_screen();
        ckprint_at(str, 0, 0, 0x0F);
        ms_sleep(ascii_to_int(temps) * 100);
        str_index = -1;
        str[0] = '\0';
    }
    return arg;
}
