/*****************************************
 * this file is the program that builds *
 *  the  disk  image  of  profan  0.9+  *
 *  You can build and run it yourself   *
 *   gcc makefsys.c -o makefsys.bin     *
 *  ./makefsys.bin "$(pwd)/out/disk"    *
 * Or you use the makefile (make disk)  *
*****************************************/

#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define SECTOR_COUNT   (1024 * 16)
#define MAX_SIZE_NAME  32
#define SECTOR_SIZE    128

#define PRINT_PROGRESS 1
#define SPEED_MODE     1  // can be dangerous
#define FREE_SECTOR    512

#define I_FILE_H 0x1
#define I_FILE   0x10
#define I_DIR    0x100
#define I_DIRCNT 0x1000
#define I_USED   0x10000

u_int32_t *virtual_disk;
u_int8_t  *free_map;
int total_sector_written;

/****************************
 * PUBLIC FUNCTIONS HEADER *
****************************/

u_int32_t fs_path_to_id(char *path);
int fs_does_path_exists(char *path);

u_int32_t fs_make_dir(char *path, char *name);
u_int32_t fs_make_file(char *path, char *name);

void *fs_declare_read_array(char *path);

void fs_write_in_file(char *path, u_int8_t *data, u_int32_t size);
void fs_read_file(char *path, char *data);

u_int32_t fs_get_file_size(char *path);
int fs_get_dir_size(char *path);
void fs_get_dir_content(char *path, u_int32_t *ids);

void fs_get_element_name(u_int32_t sector, char *name);
int fs_get_sector_type(u_int32_t sector_id);


/**********************
 * PRIVATE FUNCTIONS *
**********************/

void i_create_dir(u_int32_t sector, char *name);
void i_print_sector_smart(u_int32_t sector);

// PORT PARTIALLY
void init_fs() {
    printf("Initialisation of the filesystem...\n");
    total_sector_written = 0;
    virtual_disk = (u_int32_t *) calloc(SECTOR_COUNT * SECTOR_SIZE, sizeof(u_int32_t));
    free_map = (u_int8_t *) calloc(SECTOR_COUNT, sizeof(u_int8_t));

    printf("Done !\n");
    i_create_dir(0, "/");
}

// DO NOT PORT
void read_from_disk(u_int32_t sector, u_int32_t *buffer) {
    if (SECTOR_COUNT < sector) {
        printf("Error: sector %u is out of range\n", sector);
        exit(1);
    }
    for (int i = 0; i < SECTOR_SIZE; i++) {
        buffer[i] = virtual_disk[sector * SECTOR_SIZE + i];
    }
}

// DO NOT PORT
void write_to_disk(u_int32_t sector, u_int32_t *buffer) {
    if (SECTOR_COUNT < sector) {
        printf("Error: sector %u is out of range\n", sector);
        exit(1);
    }
    for (int i = 0; i < SECTOR_SIZE; i++) {
        virtual_disk[sector * SECTOR_SIZE + i] = buffer[i];
    }
}

void i_print_sector(u_int32_t sector) {
    u_int32_t buffer[SECTOR_SIZE];
    read_from_disk(sector, buffer);
    printf("[");
    for (int i = 0; i < SECTOR_SIZE - 1; i++) {
        printf("%x, ", buffer[i]);
    }
    printf("%x]\n", buffer[SECTOR_SIZE - 1]);
}

void i_print_sector_smart(u_int32_t sector) {
    u_int32_t buffer[SECTOR_SIZE];
    read_from_disk(sector, buffer);
    if (buffer[0] & I_FILE_H) {
        printf("[sector %d]: file header\n", sector);
        printf("    name: ");
        for (int i = 1; i < MAX_SIZE_NAME + 1; i++) {
            printf("%c", buffer[i]);
        }
        printf("\n");
        printf("    size: %d\n", buffer[MAX_SIZE_NAME + 2]);
        printf("    first sector: %d\n", buffer[SECTOR_SIZE - 1]);
    } else if (buffer[0] & I_FILE) {
        printf("[sector %d]: file content\n", sector);
        printf("    content: '");
        for (int i = 1; i < SECTOR_SIZE; i++) {
            printf("%c", buffer[i]);
        }
        printf("'\n");
    } else if (buffer[0] & I_DIR) {
        printf("[sector %d]: directory\n", sector);
        printf("    name: ");
        for (int i = 1; i < MAX_SIZE_NAME + 1; i++) {
            printf("%c", buffer[i]);
        }
        printf("\n");
        printf("    content: ");
        for (int i = MAX_SIZE_NAME + 1; i < 50; i++) {
            printf("%d ", buffer[i]);
        }
        printf("...\n");
    } else if (buffer[0] & I_DIRCNT) {
        printf("[sector %d]: directory continue\n", sector);
        printf("    content: ");
        for (int i = 0; i < 50; i++) {
            printf("%d ", buffer[i]);
        }
        printf("...\n");
    } else {
        printf("[sector %d]: unknown (0x%x)\n", sector, buffer[0]);
    }
}

void i_declare_used(u_int32_t sector) {
    if (SECTOR_COUNT < sector) {
        printf("Error: sector %u is out of range\n", sector);
        exit(1);
    }
    free_map[sector] = 1;
}

void i_declare_free(u_int32_t sector) {
    if (SECTOR_COUNT < sector) {
        printf("Error: sector %u is out of range\n", sector);
        exit(1);
    }
    free_map[sector] = 0;
}

u_int32_t i_next_free() {
    total_sector_written++;
    if (SPEED_MODE) {
        return total_sector_written;
    }
    for (int i = 0; i < SECTOR_COUNT; i++) {
        if (free_map[i] != 0) continue;
        return i;
    }
    printf("Error: no free sector (max %d)\n", SECTOR_COUNT);
    exit(1);
}

char *i_build_path(char *path, char *name) {
    int len = 2;
    for (int i = 0; path[i] != '\0'; i++) len++;
    for (int i = 0; name[i] != '\0'; i++) len++;

    char *result = calloc(len, sizeof(char));

    strcpy(result, path);
    if (path[strlen(path) - 1] != '/') {
        result[strlen(path)] = '/';
    }
    strcat(result, name);

    return result;
}

void i_create_dir(u_int32_t sector, char *name) {
    u_int32_t buffer[SECTOR_SIZE];
    read_from_disk(sector, buffer);
    if (buffer[0] & I_USED) {
        printf("Error: sector %d is already used\n", sector);
        exit(1);
    }
    if (strlen(name) > MAX_SIZE_NAME) {
        printf("Error: name is too long\n");
        exit(1);
    }
    for (int i = 0; i < SECTOR_SIZE; i++) {
        buffer[i] = 0;
    }
    buffer[0] = I_DIR | I_USED;
    for (int i = 0; i < (int) strlen(name); i++) {
        buffer[1 + i] = name[i];
    }
    i_declare_used(sector);
    write_to_disk(sector, buffer);
}

void i_create_dir_continue(u_int32_t sector) {
    u_int32_t buffer[SECTOR_SIZE];
    read_from_disk(sector, buffer);
    if (buffer[0] & I_USED) {
        printf("Error: sector %d is already used\n", sector);
        exit(1);
    }
    for (int i = 0; i < SECTOR_SIZE; i++) {
        buffer[i] = 0;
    }
    buffer[0] = I_DIRCNT | I_USED;
    i_declare_used(sector);
    write_to_disk(sector, buffer);
}

void dir_continue(u_int32_t sector) {
    u_int32_t buffer[SECTOR_SIZE];
    read_from_disk(sector, buffer);
    if (!(buffer[0] & I_USED)) {
        printf("Error: the sector isn't used\n");
        exit(1);
    }
    if (!(buffer[0] & (I_DIR | I_DIRCNT))) {
        printf("Error: the sector isn't a directory\n");
        exit(1);
    }
    u_int32_t next_sector = i_next_free();
    buffer[SECTOR_SIZE-1] = next_sector;
    write_to_disk(sector, buffer);
    i_declare_used(next_sector);

    i_create_dir_continue(next_sector);
}

void i_add_item_to_dir(u_int32_t dir_sector, u_int32_t item_sector) {
    u_int32_t buffer[SECTOR_SIZE];
    read_from_disk(dir_sector, buffer);
    if (!(buffer[0] & I_USED)) {
        printf("Error: the sector isn't used\n");
        exit(1);
    }
    if (!(buffer[0] & (I_DIR | I_DIRCNT))) {
        printf("Error: the sector isn't a directory (sector %d, flags %x)\n", dir_sector, buffer[0]);
        exit(1);
    }
    for (int i = 1 + MAX_SIZE_NAME; i < SECTOR_SIZE-1; i++) {
        if (buffer[i] == 0) {
            buffer[i] = item_sector;
            write_to_disk(dir_sector, buffer);
            return;
        }
    }
    if (buffer[SECTOR_SIZE-1] == 0) {
        u_int32_t next_sector = i_next_free();
        buffer[SECTOR_SIZE-1] = next_sector;
        write_to_disk(dir_sector, buffer);
        i_declare_used(next_sector);
        i_create_dir_continue(next_sector);
        i_add_item_to_dir(next_sector, item_sector);
    } else {
        i_add_item_to_dir(buffer[SECTOR_SIZE-1], item_sector);
    }
}

void i_remove_item_from_dir(u_int32_t dir_sector, u_int32_t item_sector) {
    u_int32_t buffer[SECTOR_SIZE];
    read_from_disk(dir_sector, buffer);
    if (!(buffer[0] & I_USED)) {
        printf("Error: the sector isn't used\n");
        exit(1);
    }
    if (!(buffer[0] & (I_DIR | I_DIRCNT))) {
        printf("Error: the sector isn't a directory\n");
        exit(1);
    }
    for (int i = 1 + MAX_SIZE_NAME; i < SECTOR_SIZE-1; i++) {
        if (buffer[i] == item_sector) {
            buffer[i] = 0;
            write_to_disk(dir_sector, buffer);
        }
    }
    if (buffer[SECTOR_SIZE-1] != 0) {
        i_remove_item_from_dir(buffer[SECTOR_SIZE-1], item_sector);
    }
    // TODO : remove empty directory continues
}

int i_get_dir_size(u_int32_t sector) {
    u_int32_t buffer[SECTOR_SIZE];
    read_from_disk(sector, buffer);
    if (!(buffer[0] & I_USED)) {
        printf("Error: the sector isn't used\n");
        exit(1);
    }
    if (!(buffer[0] & (I_DIR | I_DIRCNT))) {
        printf("Error: the sector isn't a directory\n");
        exit(1);
    }
    int result = 0;
    for (int i = 1 + MAX_SIZE_NAME; i < SECTOR_SIZE-1; i++) {
        if (buffer[i] != 0) {
            result++;
        }
    }
    if (buffer[SECTOR_SIZE-1] != 0) {
        result += i_get_dir_size(buffer[SECTOR_SIZE-1]);
    }
    return result;
}


void i_get_dir_content(u_int32_t sector, u_int32_t *ids, int index) {
    u_int32_t buffer[SECTOR_SIZE];
    read_from_disk(sector, buffer);
    if (!(buffer[0] & I_USED)) {
        printf("Error: the sector isn't used\n");
        exit(1);
    }
    if (!(buffer[0] & (I_DIR | I_DIRCNT))) {
        printf("Error: the sector isn't a directory\n");
        exit(1);
    }
    for (int i = 1 + MAX_SIZE_NAME; i < SECTOR_SIZE-1; i++) {
        if (buffer[i] != 0) {
            ids[index] = buffer[i];
            index++;
        }
    }
    if (buffer[SECTOR_SIZE-1] != 0) {
        i_get_dir_content(buffer[SECTOR_SIZE-1], ids, index);
    }
}

void i_create_file_index(u_int32_t sector, char *name) {
    u_int32_t buffer[SECTOR_SIZE];
    read_from_disk(sector, buffer);
    if (buffer[0] & I_USED) {
        printf("Error: sector %d is already used\n", sector);
        exit(1);
    }
    if (strlen(name) > MAX_SIZE_NAME) {
        printf("Error: name is too long\n");
        exit(1);
    }
    for (int i = 0; i < SECTOR_SIZE; i++) {
        buffer[i] = 0;
    }
    buffer[0] = I_FILE_H | I_USED;
    for (int i = 0; i < (int) strlen(name); i++) {
        buffer[1 + i] = name[i];
    }
    i_declare_used(sector);
    write_to_disk(sector, buffer);
}

void i_write_in_file(u_int32_t sector, u_int8_t *data, u_int32_t size) {
    u_int32_t buffer[SECTOR_SIZE];
    read_from_disk(sector, buffer);
    if (!(buffer[0] & I_USED)) {
        printf("Error: the sector isn't used\n");
        exit(1);
    }
    if (!(buffer[0] & I_FILE_H)) {
        printf("Error: the sector isn't a file header\n");
        exit(1);
    }
    u_int32_t next_sector = i_next_free();
    buffer[SECTOR_SIZE-1] = next_sector;
    buffer[MAX_SIZE_NAME + 2] = size;
    write_to_disk(sector, buffer);
    i_declare_used(next_sector);

    u_int32_t *compressed_data = malloc(sizeof(u_int32_t) * size);
    for (u_int32_t i = 0; i < size; i++) {
        compressed_data[i] = data[i];
    }

    u_int32_t sector_i, data_i;
    u_int32_t current_sector = next_sector;

    while (size > data_i) {
        buffer[0] = I_FILE | I_USED;
        for (sector_i = 1; sector_i < SECTOR_SIZE - 1; sector_i++) {
            if (size < data_i) break;
            buffer[sector_i] = compressed_data[data_i];
            data_i++;
        }
        if (size > data_i) {
            next_sector = i_next_free();
            buffer[SECTOR_SIZE-1] = next_sector;
            i_declare_used(next_sector);
        } else {
            for (int i = sector_i; i < SECTOR_SIZE-1; i++) {
                buffer[i] = 0;
            }
            buffer[SECTOR_SIZE-1] = 0;
        }
        if (data_i % 5 == 0 && PRINT_PROGRESS) {
            printf("progress: %f%%\r", (float) data_i / size * 100);
        }
        write_to_disk(current_sector, buffer);
        current_sector = next_sector;
    }

    // clean the line
    if (PRINT_PROGRESS) {
        for (int i = 0; i < 21; i++) printf(" ");
        printf("\r");
    }

    free(compressed_data);
}

char *i_read_file(u_int32_t sector) {
    u_int32_t buffer[SECTOR_SIZE];
    read_from_disk(sector, buffer);
    if (!(buffer[0] & I_USED)) {
        printf("Error: the sector isn't used\n");
        exit(1);
    }
    if (!(buffer[0] & I_FILE_H)) {
        printf("Error: the sector isn't a file header\n");
        exit(1);
    }
    char *data = malloc(buffer[MAX_SIZE_NAME + 2] * sizeof(char) + sizeof(char));
    u_int32_t *compressed_data = malloc(buffer[MAX_SIZE_NAME + 2] * (sizeof(u_int32_t) + 1));
    u_int32_t file_size = buffer[MAX_SIZE_NAME + 2];
    u_int32_t data_pointer = 0;
    sector = buffer[SECTOR_SIZE-1];
    read_from_disk(sector, buffer);
    while (sector != 0) {
        read_from_disk(sector, buffer);
        for (int i = 0; i < SECTOR_SIZE-1; i++) {
            if (data_pointer < file_size) {
                compressed_data[data_pointer] = buffer[1 + i];
                data_pointer++;
            }
        }
        sector = buffer[SECTOR_SIZE-1];
    }
    for (u_int32_t i = 0; i < file_size; i++) {
        data[i] = (char) compressed_data[i];
    }
    return data;
}

u_int32_t i_path_to_id(char *path, char *current_path, u_int32_t sector) {
    // copy the current path
    char *current_path_copy = malloc(sizeof(char) * strlen(current_path) + 1);
    strcpy(current_path_copy, current_path);

    // read the current sector
    u_int32_t buffer[SECTOR_SIZE];
    read_from_disk(sector, buffer);

    // TODO: security check

    // get the name of the current sector
    char *name = malloc(MAX_SIZE_NAME + 1 * sizeof(char));
    for (int i = 0; i < MAX_SIZE_NAME; i++) {
        name[i] = (char) buffer[1 + i];
    }

    // add the name to the current path
    strcat(current_path, name);
    free(name);
    
    if (current_path[strlen(current_path)-1] != '/') {
        strcat(current_path, "/");
    }

    // if the current path is the path we are looking for, return the sector
    if (strcmp(current_path, path) == 0) {
        free(current_path_copy);
        return sector;
    }

    // if current path is a prefix of the path we are looking for, go deeper
    if (strncmp(current_path, path, strlen(current_path)) == 0) {
        for (int i = MAX_SIZE_NAME + 1; i < SECTOR_SIZE-1; i++) {
            if (buffer[i] == 0) continue;
            
            u_int32_t result = i_path_to_id(path, current_path, buffer[i]);
            if (result == 0) continue;

            free(current_path_copy);
            return result;
        }
    }

    // if we are here, the path is not found
    strcpy(current_path, current_path_copy);
    free(current_path_copy);
    return 0;
}

/*********************
 * PUBLIC FUNCTIONS *
*********************/

u_int32_t fs_path_to_id(char *path) {
    char *edited_path = i_build_path(path, "");
    char *current_path = calloc(strlen(edited_path) + MAX_SIZE_NAME + 2, sizeof(char));

    u_int32_t exit = i_path_to_id(edited_path, current_path, 0);

    free(current_path);
    free(edited_path);
    return exit;
}

int fs_does_path_exists(char *path) {
    if (strcmp(path, "/") == 0) return 1;
    return (int) fs_path_to_id(path) != 0;
}

u_int32_t fs_make_dir(char *path, char *name) {
    char *full_name = i_build_path(path, name);
    // TODO : check if there is a directory in path
    if (fs_does_path_exists(full_name)) {
        printf("Le dossier %s existe déja !\n", full_name);
        return -1;
    }
    u_int32_t next_free = i_next_free();
    i_create_dir(next_free, name);
    i_add_item_to_dir(fs_path_to_id(path), next_free);
    free(full_name);
    return next_free;
}

u_int32_t fs_make_file(char *path, char *name) {
    char *full_name = i_build_path(path, name);

    if (fs_does_path_exists(full_name)) {
        printf("Le fichier %s existe déja !\n", full_name);
        return -1;
    }

    u_int32_t next_free = i_next_free();
    i_create_file_index(next_free, name);
    i_add_item_to_dir(fs_path_to_id(path), next_free);

    free(full_name);
    return next_free;
}

void fs_write_in_file(char *path, u_int8_t *data, u_int32_t size) {
    u_int32_t id_to_set = fs_path_to_id(path);
    i_write_in_file(id_to_set, data, size);
}

u_int32_t fs_get_file_size(char *path) {
    u_int32_t file_id = fs_path_to_id(path);
    // TODO: security check
    u_int32_t buffer[SECTOR_SIZE];
    read_from_disk(file_id, buffer);
    return buffer[MAX_SIZE_NAME + 2];
}

void *fs_declare_read_array(char *path) {
    return calloc(fs_get_file_size(path) + 1, sizeof(char));
}

// How to declare data : char *data = fs_declare_read_array(path);
// How to free data    : free(data);
void fs_read_file(char *path, char *data) {
    u_int32_t file_size = fs_get_file_size(path);
    u_int32_t data_index = 0;
    int sector = fs_path_to_id(path);
    u_int32_t buffer[SECTOR_SIZE];
    read_from_disk(sector, buffer);
    sector = buffer[SECTOR_SIZE - 1];
    while (buffer[SECTOR_SIZE-1] != 0) {
        read_from_disk(sector, buffer);
        for (int i = 1; i < SECTOR_SIZE - 1; i++) {
            if (file_size <= data_index) break;
            data[data_index] = buffer[i];
            data_index++;
        }
        sector = buffer[SECTOR_SIZE-1];
    }
    data[data_index] = '\0';
}

int fs_get_dir_size(char *path) {
    u_int32_t dir_id = fs_path_to_id(path);
    return i_get_dir_size(dir_id);
}

void fs_get_dir_content(char *path, u_int32_t *ids) {
    u_int32_t dir_id = fs_path_to_id(path);
    i_get_dir_content(dir_id, ids, 0);
}

void fs_get_element_name(u_int32_t sector, char *name) {
    u_int32_t buffer[SECTOR_SIZE];
    read_from_disk(sector, buffer);
    if (!(buffer[0] & I_USED)) {
        printf("Error: the sector isn't used\n");
        exit(1);
    }
    if (!(buffer[0] & (I_DIR | I_FILE_H))) {
        printf("Error: the sector isn't a directory or a file\n");
        exit(1);
    }
    for (int i = 0; i < MAX_SIZE_NAME; i++) {
        name[i] = buffer[i+1];
    }
}

int fs_get_sector_type(u_int32_t sector_id) {
    u_int32_t buffer[SECTOR_SIZE];
    read_from_disk(sector_id, buffer);

    if (!(buffer[0] & I_USED))  return 0; // sector empty
    if (buffer[0] & I_FILE)     return 1; // file content
    if (buffer[0] & I_FILE_H)   return 2; // file index
    if (buffer[0] & I_DIR)      return 3; // dir
    if (buffer[0] & I_DIRCNT)   return 4; // dir continue
    return 0;                             // sector empty
}

// TODO : add a function to delete a file
// TODO : add a function to delete a directory

/******************
 * DISK TRANSFER *
******************/

void send_file_to_disk(char *linux_path, char *parent, char *name) {
    char *profan_path = i_build_path(parent, name);

    fs_make_file(parent, name);

    // get the file content
    FILE *f = fopen(linux_path, "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    u_int8_t *file_content = malloc(fsize + 1);
    fread(file_content, fsize, 1, f);
    fclose(f);
    file_content[fsize] = '\0';

    fs_write_in_file(profan_path, file_content, fsize);

    free(file_content);
    free(profan_path);
}

void arboresence_to_disk(char *linux_path, char *parent, char *name) {
    DIR *d = opendir(linux_path); // open the path

    if (d == NULL) {
        printf("Cannot open directory %s\n", linux_path);
        return;
    }

    char *profan_path = i_build_path(parent, name);

    if (strcmp(name, "")) {
        printf("| make dir  %s\n", profan_path);
        fs_make_dir(parent, name);
    } else {
        printf("STARTING DISK TRANSFER\n");
    }

    // list all the files and directories within directory
    struct dirent *dir;
    while ((dir = readdir(d)) != NULL) {
        if (dir->d_type == DT_DIR) {
            // Found a directory, but ignore . and ..
            if (!strcmp(dir->d_name, ".") || !strcmp(dir->d_name, "..")) continue;
            char *new_linux_path = i_build_path(linux_path, dir->d_name);

            arboresence_to_disk(new_linux_path, profan_path, dir->d_name);
            free(new_linux_path);
        } else {
            // get the file content
            char *file_path = i_build_path(linux_path, dir->d_name);
            char *printable_path = i_build_path(profan_path, dir->d_name);
            
            printf("| make file %s\n", printable_path);
            send_file_to_disk(file_path, profan_path, dir->d_name);

            free(printable_path);
            free(file_path);
        }
    }
    free(profan_path);
    closedir(d);
}

void put_in_disk() {
    FILE *fptr;
    if ((fptr = fopen("HDD.bin","wb")) == NULL) {
       printf("Error! opening file");
       exit(1);
    }

    int i, to_write = total_sector_written + FREE_SECTOR + 1;
    for (i = 0; i < to_write; i++) {
        u_int32_t buffer[SECTOR_SIZE];
        read_from_disk(i, buffer);
        fwrite(buffer, sizeof(u_int32_t), SECTOR_SIZE, fptr);
    }
    printf("put in disk done, %d sectors written (%d used)\n", to_write, total_sector_written);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage : %s <path>\n", argv[0]);
        return 1;
    }

    init_fs();
    arboresence_to_disk(argv[1], "/", "");

    put_in_disk();

    return 0;
}
