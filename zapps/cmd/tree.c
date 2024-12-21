/*****************************************************************************\
|   === tree.c : 2024 ===                                                     |
|                                                                             |
|    Draws a tree of the directory structure                       .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <profan/filesys.h>
#include <profan.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define CHAR_VERT   0xB3
#define CHAR_HORI   0xC4
#define CHAR_CROSS  0xC3
#define CHAR_CORNER 0xC0

char is_last[256];

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

void sort_alpha_and_type(int count, char **names, uint32_t *ids) {
    char *tmp_name;
    uint32_t tmp_id;
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

int draw_tree(uint32_t sid, int depth) {
    // get the directory content
    char **names;
    uint32_t *sids;
    int count;

    count = fu_get_dir_content(sid, &sids, &names);

    if (count == -1) {
        return 1;
    }

    // sort the content
    sort_alpha_and_type(count, names, sids);

    is_last[depth] = 0;

    // search for the path part
    for (int j = 0; j < count; j++) {
        if (strcmp(names[j], ".") == 0 || strcmp(names[j], "..") == 0)
            continue;
        if (j == count - 1)
            is_last[depth] = 1;
        for (int i = 0; i < depth; i++) {
            if (is_last[i]) {
                printf("    ");
            } else {
                printf("%c   ", CHAR_VERT);
            }
        }
        if (is_last[depth]) {
            printf("%c%c%c ", CHAR_CORNER, CHAR_HORI, CHAR_HORI);
        } else {
            printf("%c%c%c ", CHAR_CROSS, CHAR_HORI, CHAR_HORI);
        }
        if (fu_is_dir(sids[j])) {
            printf("\033[94m%s\033[0m\n", names[j]);
            draw_tree(sids[j], depth + 1);
        } else {
            printf("%s\n", names[j]);
        }
    }

    // free
    for (int j = 0; j < count; j++)
        profan_kfree(names[j]);
    profan_kfree(names);
    profan_kfree(sids);

    return 0;
}

int main(int argc, char **argv) {
    if (argc > 2 || (argv[1] && argv[1][0] == '-')) {
        fprintf(stderr, "Usage: %s [path]\n", argv[0]);
        return 1;
    }

    const char *path = argv[1] ? argv[1] : profan_wd_path;
    uint32_t sid = profan_resolve_path(path);

    if (!fu_is_dir(sid)) {
        fprintf(stderr, "Error: %s: Not a directory\n", path);
        return 1;
    }

    printf("\033[94m%s\033[0m\n", path);
    draw_tree(sid, 0);

    return 0;
}
