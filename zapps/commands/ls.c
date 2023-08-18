#include <syscall.h>
#include <filesys.h>
#include <stdlib.h>
#include <profan.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char **argv) {
    char *ls_path = malloc(256);

    if (argc > 2) {
        assemble_path(argv[1], argv[2], ls_path);
    } else strcpy(ls_path, argv[1]);

    sid_t dir = fu_path_to_sid(ROOT_SID, ls_path);

    if (!IS_NULL_SID(dir) && fu_is_file(dir)) {
        printf("$3%s$B is not a directory\n", ls_path);
        free(ls_path);
        return 1;
    }

    sid_t *cnt_ids;
    char **cnt_names;

    int elm_count = fu_get_dir_content(dir, &cnt_ids, &cnt_names);

    if (!elm_count) {
        free(ls_path);
        return 0;
    }

    for (int i = 0; i < elm_count; i++) {
        if (fu_is_dir(cnt_ids[i])) {
            printf("$2%s", cnt_names[i]);
            for (unsigned int j = 0; j < 22 - strlen(cnt_names[i]); j++) c_kprint(" ");
            printf("%d elm\n", fu_get_dir_content(cnt_ids[i], NULL, NULL));
            free(cnt_names[i]);
        }
    }

    int size;
    for (int i = 0; i < elm_count; i++) {
        if (fu_is_file(cnt_ids[i])) {
            printf("$1%s", cnt_names[i]);
            for (unsigned int j = 0; j < 22 - strlen(cnt_names[i]); j++) c_kprint(" ");
            size = fu_get_file_size(cnt_ids[i]);
            if (size < 1024) printf("%d oct\n", size);
            else if (size < 1024 * 1024) printf("%d.%d Ko\n", size / 1024, (size % 1024) / 10);
            else printf("%d.%d Mo\n", size / (1024 * 1024), (size % (1024 * 1024)) / 10);
            free(cnt_names[i]);
        }
    }

    free(cnt_names);
    free(cnt_ids);

    free(ls_path);
    return 0;
}
