#include <filesys.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Usage: mklink <fullpath>\n");
        return 1;
    }

    char *val;
    
    sid_t lk = fu_link_create(0, argv[1]);
    printf("fu_link_create: d%ds%d\n", lk.device, lk.sector);

    printf("fu_link_add_path pid 66: %d\n", fu_link_add_path(lk, 66, "coucou"));
    printf("fu_link_add_path pid 67: %d\n", fu_link_add_path(lk, 67, "salut"));
    
    val = fu_link_get_path(lk, 66);
    printf("fu_link_get_path pid 66: %s\n", val);
    free(val);

    val = fu_link_get_path(lk, 67);
    printf("fu_link_get_path pid 67: %s\n", val);
    free(val);

    return 0;
}
