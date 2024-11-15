/*****************************************************************************\
|   === hostio.c : 2024 ===                                                   |
|                                                                             |
|    Part of the filesystem creation tool                          .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include "../butterfly.h"
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>

int save_vdisk(vdisk_t *vdisk, char *filename) {
    // we need to create a file and write the vdisk to it
    FILE *fp = fopen(filename, "wb");
    if (fp == NULL) {
        printf("error: could not open file %s\n", filename);
        return -1;
    }
    // get the last used sector
    uint32_t last_sector = 0;
    for (uint32_t i = 0; i < vdisk->size; i++) {
        if (vdisk->sectors[i]->data[0]) {
            last_sector = i;
        }
    }
    printf("saving vdisk to %s, last_sector: %d\n", filename, last_sector);

    for (uint32_t i = 0; i <= last_sector; i++) {
        fwrite(vdisk->sectors[i]->data, 1, SECTOR_SIZE, fp);
    }

    // close the file
    fclose(fp);
    return 0;
}

vdisk_t *load_vdisk(char *filename, uint32_t min_size) {
    // we need to read the file and write it to the vdisk
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) {
        printf("error: could not open file %s\n", filename);
        return NULL;
    }
    // get the file size
    uint32_t size;
    fseek(fp, 0, SEEK_END);
    size = ftell(fp) / SECTOR_SIZE;
    fseek(fp, 0, SEEK_SET);
    printf("loading vdisk from %s, size: %d sectors\n", filename, size);

    // create the vdisk
    vdisk_t *vdisk = vdisk_create(max(size, min_size));
    if (vdisk == NULL) {
        printf("error: could not create vdisk\n");
        return NULL;
    }

    // read the file
    for (uint32_t i = 0; i < size; i++) {
        fread(vdisk->sectors[i]->data, 1, SECTOR_SIZE, fp);
        if (vdisk->sectors[i]->data[0]) {
            vdisk_note_sector_used(vdisk, i);
        }
    }

    // close the file
    fclose(fp);
    return vdisk;
}

void path_join(char *dst, char *a, char *b) {
    strcpy(dst, a);
    if (dst[strlen(dst) - 1] != '/') {
        strcat(dst, "/");
    }
    strcat(dst, b);
}

int host_to_internal(filesys_t *filesys, char *extern_path, char *intern_path) {
    // list the files in the directory
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(extern_path)) != NULL) {
        // iterate over the files in the directory
        while ((ent = readdir(dir)) != NULL) {
            // skip . and ..
            if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..")) {
                continue;
            }
            // create the file or directory
            char *path = malloc(strlen(extern_path) + strlen(ent->d_name) + 2);
            path_join(path, extern_path, ent->d_name);
            char *intern_path2 = malloc(strlen(intern_path) + strlen(ent->d_name) + 2);
            path_join(intern_path2, intern_path, ent->d_name);
            if (ent->d_type == DT_DIR) {
                // create the directory
                fu_dir_create(filesys, 0, intern_path2);

                // recursively call host_to_internal
                host_to_internal(filesys, path, intern_path2);
            } else {
                // create the file
                fu_file_create(filesys, 0, intern_path2);
                // get the file size
                FILE *fp = fopen(path, "rb");
                if (fp == NULL) {
                    printf("could not open file %s\n", path);
                    return -1;
                }
                fseek(fp, 0, SEEK_END);
                uint32_t size = ftell(fp);
                fseek(fp, 0, SEEK_SET);
                // set the file size
                fs_cnt_set_size(filesys, fu_path_to_sid(filesys, SID_ROOT, intern_path2), size);
                // read the file
                uint8_t *buf = malloc(size);
                fread(buf, 1, size, fp);
                // write the file
                fs_cnt_write(filesys, fu_path_to_sid(filesys, SID_ROOT, intern_path2), buf, 0, size);
                // close the file
                fclose(fp);
                free(buf);
            }
            free(intern_path2);
            free(path);
        }
        closedir(dir);
    } else {
        // could not open directory
        printf("error: could not open directory %s\n", extern_path);
        return -1;
    }
    return 0;
}

void create_if_not_exists(char *path) {
    DIR *dir = opendir(path);
    if (dir) {
        // directory exists
        closedir(dir);
    } else if (ENOENT == errno) {
        // directory does not exist
        mkdir(path, 0777);
    } else {
        // could not open directory
        printf("error: could not open directory %s\n", path);
    }
}

int internal_to_host(filesys_t *filesys, char *extern_path, char *intern_path) {
    create_if_not_exists(extern_path);

    char **names;
    uint32_t *sids;

    int count = fu_get_dir_content(
        filesys,
        fu_path_to_sid(filesys, SID_ROOT, intern_path),
        &sids,
        &names
    );

    if (count < 0) {
        printf("error: could not get directory content\n");
        return -1;
    }

    for (int i = 0; i < count; i++) {
        char *path = malloc(strlen(extern_path) + strlen(names[i]) + 2);
        path_join(path, extern_path, names[i]);
        char *intern_path2 = malloc(strlen(intern_path) + strlen(names[i]) + 2);
        path_join(intern_path2, intern_path, names[i]);
        if (fu_is_file(filesys, sids[i])) {
            // create the file
            FILE *fp = fopen(path, "wb");
            if (fp == NULL) {
                printf("error: could not open file %s\n", path);
                return -1;
            }
            // get the file size
            uint32_t size = fs_cnt_get_size(filesys, sids[i]);
            // read the file
            uint8_t *buf = malloc(size);
            if (fs_cnt_read(filesys, sids[i], buf, 0, size)) {
                printf("error: could not read file %s\n", path);
                return -1;
            }
            // write the file
            fwrite(buf, 1, size, fp);
            // close the file
            fclose(fp);
            free(buf);
        } else {
            // create the directory
            mkdir(path, 0777);
            // recursively call internal_to_host
            internal_to_host(filesys, path, intern_path2);
        }
        free(intern_path2);
        free(path);
    }

    for (int i = 0; i < count; i++) {
        free(names[i]);
    }
    free(names);
    free(sids);

    return 0;
}
