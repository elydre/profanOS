#include <filesys.h>
#include <stdio.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Usage: mklink <fullpath>\n");
        return 1;
    }
    
    sid_t lk = fu_link_create(0, argv[1]);
    printf("fu_link_create: d%ds%d\n", lk.device, lk.sector);

    printf("fu_link_add_path pid 1: %d\n", fu_link_add_path(lk, 1, "coucou"));
    printf("fu_link_add_path pid 2: %d\n", fu_link_add_path(lk, 2, "salut"));

    printf("fu_link_get_path pid 1: %s\n", fu_link_get_path(lk, 1));
    printf("fu_link_get_path pid 2: %s\n", fu_link_get_path(lk, 2));

    return 0;
}
