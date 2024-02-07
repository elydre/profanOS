#include <filesys.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char **argv) {
    if (!argv[1])
        puts("Usage: debug <-p|-c>");
    else if (!strcmp(argv[1], "-p"))
        fm_debug(-1);
    else if (!strcmp(argv[1], "-c"))
        fm_clean();
    else
        puts("Usage: debug <-p|-c>");

    return 0;
}
