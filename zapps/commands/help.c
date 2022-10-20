#include <syscall.h>

void assemble_path(char old[], char new[], char result[]);

int main(int argc, char **argv) {
    if (argc > 2) {
        c_fskprint("$BSpecialised help is not yet implemented.\n");
        return 0;
    }
    char *file = "/zada/shell_help.txt";
    if (c_fs_does_path_exists(file) && c_fs_type_sector(c_fs_path_to_id(file, 0)) == 2) {
        uint32_t * file_content = c_fs_declare_read_array(file);
        char * char_content = c_fs_declare_read_array(file);
        c_fs_read_file(file, file_content);
        int char_count;
        for (char_count = 0; file_content[char_count] != (uint32_t) -1; char_count++)
            char_content[char_count] = (char) file_content[char_count];
        char_content[char_count] = '\0';
        c_ckprint(char_content, c_magenta);
        c_free(file_content);
        c_free(char_content);
    } else c_fskprint("$Bcommand not found\n");
    c_free(file);
    return 0;
}
