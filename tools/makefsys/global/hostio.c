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

int hio_raw_import(const char *filename) {
    FILE *fp = fopen(filename, "rb");
    
    if (fp == NULL) {
        printf("error: could not open file %s\n", filename);
        return 1;
    }

    // cleanup the vdisk
    vdisk_destroy();
    vdisk_init();

    // read the file and write it to the vdisk
    uint8_t buffer[SECTOR_SIZE];

    size_t bytes_read;
    for (uint32_t sector = 0; (bytes_read = fread(buffer, 1, SECTOR_SIZE, fp)) > 0; sector++) {
        if (vdisk_write(buffer, bytes_read, sector * SECTOR_SIZE)) {
            printf("error: could not write to vdisk at sector %d\n", sector);
            fclose(fp);
            return 1;
        }
    }

    // close the file
    fclose(fp);
    return 0;
}

int hio_raw_export(const char *filename) {
    FILE *fp = fopen(filename, "wb");
    if (fp == NULL) {
        printf("error: could not open file %s\n", filename);
        return 1;
    }

    // write the vdisk to the file
    uint8_t buffer[SECTOR_SIZE];
    for (uint32_t sector = 0; vdisk_read(buffer, SECTOR_SIZE, sector * SECTOR_SIZE) == 0; sector++) {
        size_t bytes_written = fwrite(buffer, 1, SECTOR_SIZE, fp);
        if (bytes_written < SECTOR_SIZE) {
            printf("error: could not write to file %s\n", filename);
            fclose(fp);
            return 1;
        }
    }

    // close the file
    fclose(fp);
    return 0;
}

static void path_join(char *dst, const char *a, char *b) {
    strcpy(dst, a);
    if (dst[strlen(dst) - 1] != '/')
        strcat(dst, "/");

    strcat(dst, b);
}

int hio_dir_import(const char *extern_path, const char *intern_path) {
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
            if (ent->d_type == DT_DIR) {
                // create the directory
                fu_dir_create(0, intern_path, ent->d_name);

                // recursively call hio_dir_import
                char *intern_path2 = malloc(strlen(intern_path) + strlen(ent->d_name) + 2);
                path_join(intern_path2, intern_path, ent->d_name);
            
                hio_dir_import(path, intern_path2);

                free(intern_path2);
            } else {
                // create the file
                sid_t sid = fu_file_create(intern_path, ent->d_name);
                // get the file size
                FILE *fp = fopen(path, "rb");
                if (fp == NULL) {
                    printf("could not open file %s\n", path);
                    return 1;
                }
                fseek(fp, 0, SEEK_END);
                uint32_t size = ftell(fp);
                fseek(fp, 0, SEEK_SET);
                // set the file size
                fs_cnt_set_size(sid, size);
                // read the file
                uint8_t *buf = malloc(size);
                fread(buf, 1, size, fp);
                // write the file
                fs_cnt_write(sid, buf, 0, size);
                // close the file
                fclose(fp);
                free(buf);
            }
            free(path);
        }
        closedir(dir);
    } else {
        // could not open directory
        printf("error: could not open directory %s\n", extern_path);
        return 1;
    }
    return 0;
}

static void create_if_not_exists(const char *path) {
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

int hio_dir_export(const char *extern_path, const char *intern_path) {
    create_if_not_exists(extern_path);

    char **names;
    uint32_t *sids;

    int count = fu_dir_get_content(
        fu_path_to_sid(SID_ROOT, intern_path),
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
        if (fu_is_file(sids[i])) {
            // create the file
            FILE *fp = fopen(path, "wb");
            if (fp == NULL) {
                printf("error: could not open file %s\n", path);
                return 1;
            }
            // get the file size
            uint32_t size = fs_cnt_get_size(sids[i]);
            // read the file
            uint8_t *buf = malloc(size);
            if (fs_cnt_read(sids[i], buf, 0, size)) {
                printf("error: could not read file %s\n", path);
                return 1;
            }
            // write the file
            fwrite(buf, 1, size, fp);
            // close the file
            fclose(fp);
            free(buf);
        } else {
            // create the directory
            mkdir(path, 0777);
            // recursively call hio_dir_export
            hio_dir_export(path, intern_path2);
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
