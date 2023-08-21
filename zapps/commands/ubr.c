#include <filesys.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char **argv) {
    if (argc != 4) {
        printf("Usage: ubr <buffer name> <redirection file>\n");
        return 1;
    }

    char *buffer_name = argv[2];
    char *redirection_file = argv[3];

    uint32_t buffer_id = 0;
    if (strcmp(buffer_name, "stdout") == 0) {
        buffer_id = DEVIO_STDOUT;
    } else if (strcmp(buffer_name, "stderr") == 0) {
        buffer_id = DEVIO_STDERR;
    } else if (strcmp(buffer_name, "buffer") == 0) {
        buffer_id = DEVIO_BUFFER;
    } else {
        printf("Invalid buffer name: %s, must be one of stdout, stderr, buffer\n", buffer_name);
        return 1;
    }

    sid_t redirection_sid = fu_path_to_sid(ROOT_SID, redirection_file);

    if (IS_NULL_SID(redirection_sid)) {
        printf("Invalid redirection file: %s\n", redirection_file);
        return 1;
    }

    if (!(fu_is_file(redirection_sid) || fu_is_fctf(redirection_sid))) {
        printf("Redirection file must be a file or a FCTF\n");
        return 1;
    }

    if (devio_change_redirection(buffer_id, redirection_sid)) {
        printf("Failed to change redirection\n");
        return 1;
    }

    return 0;
}
