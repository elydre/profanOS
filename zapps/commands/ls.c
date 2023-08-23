#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <syscall.h>
#include <filesys.h>
#include <profan.h>


int cmp_string_alpha(char *s1, char *s2) {
    int i = 0;
    while (s1[i] && s2[i]) {
        if (s1[i] < s2[i]) return -1;
        else if (s1[i] > s2[i]) return 1;
        i++;
    }
    if (s1[i]) return 1;
    else if (s2[i]) return -1;
    else return 0;
}

void sort_alpha_and_type(int count, char **names, sid_t *ids) {
    char *tmp_name;
    sid_t tmp_id;
    int i, j;

    int dir_count = 0;

    // put directories first
    for (i = 0; i < count; i++) {
        if (fu_is_dir(ids[i])) {
            tmp_name = names[dir_count];
            names[dir_count] = names[i];
            names[i] = tmp_name;

            tmp_id = ids[dir_count];
            ids[dir_count] = ids[i];
            ids[i] = tmp_id;

            dir_count++;
        }
    }

    // sort directories
    for (i = 0; i < dir_count; i++) {
        for (j = i + 1; j < dir_count; j++) {
            if (cmp_string_alpha(names[i], names[j]) > 0) {
                tmp_name = names[i];
                names[i] = names[j];
                names[j] = tmp_name;

                tmp_id = ids[i];
                ids[i] = ids[j];
                ids[j] = tmp_id;
            }
        }
    }

    // sort files
    for (i = dir_count; i < count; i++) {
        for (j = i + 1; j < count; j++) {
            if (cmp_string_alpha(names[i], names[j]) > 0) {
                tmp_name = names[i];
                names[i] = names[j];
                names[j] = tmp_name;

                tmp_id = ids[i];
                ids[i] = ids[j];
                ids[j] = tmp_id;
            }
        }
    }
}

int main(int argc, char **argv) {
    char *ls_path = malloc(256);

    if (argc > 2) {
        assemble_path(argv[1], argv[2], ls_path);
    } else strcpy(ls_path, argv[1]);

    sid_t dir = fu_path_to_sid(ROOT_SID, ls_path);

    if (IS_NULL_SID(dir) || !fu_is_dir(dir)) {
        printf("$3%s$B is not a directory$$\n", ls_path);
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

    sort_alpha_and_type(elm_count, cnt_names, cnt_ids);

    int size;

    for (int i = 0; i < elm_count; i++) {
        if (fu_is_dir(cnt_ids[i])) {
            printf("$2%s$$", cnt_names[i]);
            for (unsigned int j = 0; j < 22 - strlen(cnt_names[i]); j++) putchar(' ');
            printf("%d elm\n", fu_get_dir_content(cnt_ids[i], NULL, NULL));
        } else if (fu_is_file(cnt_ids[i])) {
            printf("$1%s$$", cnt_names[i]);
            for (unsigned int j = 0; j < 22 - strlen(cnt_names[i]); j++) putchar(' ');
            size = fu_get_file_size(cnt_ids[i]);
            if (size < 1024) printf("%d oct\n", size);
            else if (size < 1024 * 1024) printf("%d.%d Ko\n", size / 1024, (size % 1024) / 10);
            else printf("%d.%d Mo\n", size / (1024 * 1024), (size % (1024 * 1024)) / 10);
        } else if (fu_is_fctf(cnt_ids[i])) {
            printf("$5%s$$", cnt_names[i]);
            for (unsigned int j = 0; j < 22 - strlen(cnt_names[i]); j++) putchar(' ');
            printf("fctf: %p\n", fu_fctf_get_addr(cnt_ids[i]));
        } else {
            printf("$3%s$$", cnt_names[i]);
            for (unsigned int j = 0; j < 22 - strlen(cnt_names[i]); j++) putchar(' ');
            printf("unk\n");
        }
        free(cnt_names[i]);
    }

    free(cnt_names);
    free(cnt_ids);

    free(ls_path);
    return 0;
}
