// requires /user/star_wars.txt, the ascii art of the star wars

#include <syscall.h>
#include <iolib.h>


int main(int argc, char **argv) {

    char path[] = "/zada/star_wars.txt";

    fskprint("allocating memory for the file...\n");
    uint32_t *data = c_fs_declare_read_array(path);
    char *str = c_malloc(0x1000);

    str[0] = '\0';

    fskprint("loading file: %s into memory...\n", path);
    c_fs_read_file(path, data);

    c_cursor_blink(1);
    c_clear_screen();

    int line = 0, str_index = 0, j;

    char temps[5];

    for (int i = 0; c_kb_get_scancode() != 1; i++) {
        str_index++;
        str[str_index] = (char) data[i];
        if (str[str_index] != '\n') continue;
        line++;
        if (line % 14) continue;
        for (j = 0; str[j] > 40; j++) temps[j] = str[j];
        temps[j] = '\0';
        str[str_index] = '\0';
        if (c_ascii_to_int(temps) < 0) break;
        c_clear_screen();
        c_ckprint_at(str, 0, 0, 0x0F);
        c_ms_sleep(c_ascii_to_int(temps) * 100);
        str_index = -1;
        str[0] = '\0';
    }

    c_clear_screen();
    c_free(data);
    c_free(str);
    c_cursor_blink(0);
    return 0;
}
