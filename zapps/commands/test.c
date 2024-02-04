#include <filesys.h>
#include <stdio.h>

int main(void) {
    fu_file_create(0, "/test.txt");

    int fd = fm_open("/test.txt");
    if (fd < 0)
        return 1;

    printf("File opened with fd: %d\n", fd);

    printf("Writing : %d\n", fm_write(fd, "Hello, World!\n", 14));
    printf("Writing : %d\n", fm_write(fd, "oui\n", 4));

    printf("Writing (fd 1): %d\n", fm_write(1, "in stdout\n", 10));

    fm_dup2(fd, 1);

    printf("Writing (fd 1): %d\n", fm_write(1, "in stdout\n", 10));

    fm_close(fd);

    return 0;
}
