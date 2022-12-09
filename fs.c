#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define SECTOR_COUNT 1024
#define SECTOR_SIZE 128
#define MAX_SIZE_NAME 32
#define I_FILE_HEADER 0x1
#define I_FILE 0x10
#define I_DIRECTORY 0x100
#define I_DIRECTORY_CONTINUE 0x1000
#define I_USED 0x10000

u_int32_t *virtual_disk;
u_int8_t *free_map;

void i_create_dir(u_int32_t sector, char *name);

// PORT PARTIALLY
void init_fs() {
    printf("Initialisation of the filesystem...\n");
    virtual_disk = (u_int32_t *) malloc(SECTOR_COUNT * SECTOR_SIZE * sizeof(u_int32_t));
    for (int i = 0; i < SECTOR_COUNT * SECTOR_SIZE; i++) {
        virtual_disk[i] = 0;
    }
    free_map = (u_int8_t *) malloc(SECTOR_COUNT * sizeof(u_int8_t));
    for (int i = 0; i < SECTOR_COUNT; i++) {
        free_map[i] = 0;
    }
    printf("Done !\n");
    i_create_dir(0, "/");
}

// DO NOT PORT
void read_from_disk(u_int32_t sector, u_int32_t *buffer) {
    for (int i = 0; i < SECTOR_SIZE; i++) {
        buffer[i] = virtual_disk[sector * SECTOR_SIZE + i];
    }
}

// DO NOT PORT
void write_to_disk(u_int32_t sector, u_int32_t *buffer) {
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

void declare_used(u_int32_t sector) {
    free_map[sector] = 1;
}

void declare_free(u_int32_t sector) {
    free_map[sector] = 0;
}

u_int32_t i_next_free() {
    for (int i = 0; i < SECTOR_COUNT; i++) {
        if (free_map[i] != 0) continue;
        return i;
    }
    return -1;
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
    buffer[0] = I_DIRECTORY | I_USED;
    for (int i = 0; i < strlen(name); i++) {
        buffer[1 + i] = name[i];
    }
    declare_used(sector);
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
    buffer[0] = I_DIRECTORY_CONTINUE | I_USED;
    declare_used(sector);
    write_to_disk(sector, buffer);
}

void dir_continue(u_int32_t sector) {
    u_int32_t buffer[SECTOR_SIZE];
    read_from_disk(sector, buffer);
    if (!(buffer[0] & I_USED)) {
        printf("Error: the sector isn't used\n");
        exit(1);
    }
    if (!(buffer[0] & (I_DIRECTORY | I_DIRECTORY_CONTINUE))) {
        printf("Error: the sector isn't a directory\n");
        exit(1);
    }
    u_int32_t next_sector = i_next_free();
    buffer[SECTOR_SIZE-1] = next_sector;
    write_to_disk(sector, buffer);
    declare_used(next_sector);

    i_create_dir_continue(next_sector);
}

void i_add_item_to_dir(u_int32_t dir_sector, u_int32_t item_sector) {
    u_int32_t buffer[SECTOR_SIZE];
    read_from_disk(dir_sector, buffer);
    if (!(buffer[0] & I_USED)) {
        printf("Error: the sector isn't used\n");
        exit(1);
    }
    if (!(buffer[0] & (I_DIRECTORY | I_DIRECTORY_CONTINUE))) {
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
        declare_used(next_sector);
        i_create_dir_continue(next_sector);
        i_add_item_to_dir(next_sector, item_sector);
    } else {
        i_add_item_to_dir(buffer[SECTOR_SIZE-1], item_sector);
    }
}

void remove_item_from_dir(u_int32_t dir_sector, u_int32_t item_sector) {
    u_int32_t buffer[SECTOR_SIZE];
    read_from_disk(dir_sector, buffer);
    if (!(buffer[0] & I_USED)) {
        printf("Error: the sector isn't used\n");
        exit(1);
    }
    if (!(buffer[0] & (I_DIRECTORY | I_DIRECTORY_CONTINUE))) {
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
        remove_item_from_dir(buffer[SECTOR_SIZE-1], item_sector);
    }
    // TODO : remove empty directory continues
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
    buffer[0] = I_FILE_HEADER | I_USED;
    for (int i = 0; i < strlen(name); i++) {
        buffer[1 + i] = name[i];
    }
    declare_used(sector);
    write_to_disk(sector, buffer);
}

void i_writefile(u_int32_t sector, u_int32_t *data, int data_pointer, int max_size) {
    u_int32_t buffer[SECTOR_SIZE];
    read_from_disk(sector, buffer);
    buffer[0] = I_FILE | I_USED;
    for (int i = 0; i < SECTOR_SIZE-1; i++) {
        if (data_pointer < max_size) {
            buffer[1 + i] = data[data_pointer];
            data_pointer++;
        } else {
            buffer[1 + i] = 0;
        }
    }
    write_to_disk(sector, buffer);
    if (data_pointer < max_size) {
        u_int32_t next_sector = i_next_free();
        buffer[SECTOR_SIZE-1] = next_sector;
        write_to_disk(sector, buffer);
        declare_used(next_sector);
        i_writefile(next_sector, data, data_pointer, max_size);
    }
}

void i_write_in_file(u_int32_t sector, char *data) {
    u_int32_t buffer[SECTOR_SIZE];
    read_from_disk(sector, buffer);
    if (!(buffer[0] & I_USED)) {
        printf("Error: the sector isn't used\n");
        exit(1);
    }
    if (!(buffer[0] & I_FILE_HEADER)) {
        printf("Error: the sector isn't a file header\n");
        exit(1);
    }
    u_int32_t next_sector = i_next_free();
    buffer[SECTOR_SIZE-1] = next_sector;
    buffer[1 + MAX_SIZE_NAME + 1] = (u_int32_t) strlen(data);
    write_to_disk(sector, buffer);
    declare_used(next_sector);
    u_int32_t *compressed_data = malloc(sizeof(u_int32_t) * strlen(data));
    for (int i = 0; i < strlen(data); i++) {
        compressed_data[i] = data[i];
    }
    for (int i = 1; i < SECTOR_SIZE-1; i++) {
        if (i < strlen(data)) {
            buffer[i] = compressed_data[i];
        } else {
            buffer[i] = 0;
        }
    }
    write_to_disk(next_sector, buffer);
    i_writefile(next_sector, compressed_data, 0, strlen(data));
    free(compressed_data);
}

char *i_read_file(u_int32_t sector) {
    u_int32_t buffer[SECTOR_SIZE];
    read_from_disk(sector, buffer);
    if (!(buffer[0] & I_USED)) {
        printf("Error: the sector isn't used\n");
        exit(1);
    }
    if (!(buffer[0] & I_FILE_HEADER)) {
        printf("Error: the sector isn't a file header\n");
        exit(1);
    }
    char *data = malloc(buffer[1 + MAX_SIZE_NAME + 1] * sizeof(char) + sizeof(char));
    u_int32_t *compressed_data = malloc(buffer[1 + MAX_SIZE_NAME + 1] * (sizeof(u_int32_t) + 1));
    u_int32_t file_size = buffer[1 + MAX_SIZE_NAME + 1];
    int data_pointer = 0;
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
    for (int i = 0; i < file_size; i++) {
        data[i] = (char) compressed_data[i];
    }
    return data;
}

u_int32_t path_to_id(char *path, char *current_path, u_int32_t sector) {
    // printf("[new] path_to_id('%s', '%s', '%d')\n", path, current_path, sector);

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
            
            u_int32_t result = path_to_id(path, current_path, buffer[i]);
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

// PUBLIC FUNCTIONS

u_int32_t fs_path_to_id(char *path) {
    char *edited_path = malloc(sizeof(char) * (strlen(path) + 2));
    strcpy(edited_path, path);

    // check if the path ends with a '/'
    if (edited_path[strlen(edited_path) - 1] != '/') {
        // if not, add it
        strcat(edited_path, "/");
    }

    char *current_path = calloc(strlen(edited_path) + 1, sizeof(char));

    u_int32_t exit = path_to_id(edited_path, current_path, 0);

    free(current_path);
    free(edited_path);
    return exit;
}

int fs_does_path_exists(char *path) {
    if (strcmp(path, "/") == 0) {
        return 1;
    } else {
        return (int) fs_path_to_id(path) != 0;
    }
}

u_int32_t fs_make_dir(char *path, char *name) {
    char *full_name = malloc(strlen(path) + strlen(name) + 1);
    strcpy(full_name, path);
    if (strcmp("/", path)) {
        full_name[strlen(path)] = '/';
        for (int i = 0; i < strlen(name); i++) {
            full_name[strlen(path) + i + 1] = name[i];
        }
        full_name[strlen(path)+strlen(name) + 1] = '\0';
    } else {
        for (int i = 0; i < strlen(name); i++) {
            full_name[strlen(path) + i] = name[i];
        }
        full_name[strlen(path)+strlen(name)] = '\0';
    }
    // TODO : check if there is a directory in path
    if (fs_does_path_exists(full_name)) {
        printf("Le dossier %s existe déja !\n", full_name);
        return -1;
    }
    printf("Creating dir %s...\n", full_name);
    u_int32_t next_free = i_next_free();
    i_create_dir(next_free, name);
    i_add_item_to_dir(fs_path_to_id(path), next_free);
    free(full_name);
    return next_free;
}

u_int32_t fs_make_file(char path[], char name[]) {
    char *full_name = malloc(strlen(path) + strlen(name) + 1);
    strcpy(full_name, path);
    full_name[strlen(path)] = '/';
    for (int i = 0; i < strlen(name); i++) {
        full_name[strlen(path) + i + 1] = name[i];
    }
    full_name[strlen(path)+strlen(name) + 1] = '\0';
    if (fs_does_path_exists(full_name)) {
        printf("Le fichier %s existe déja !\n", full_name);
        return -1;
    }
    printf("Creating file %s...\n", full_name);
    u_int32_t next_free = i_next_free();
    i_create_file_index(next_free, name);
    i_add_item_to_dir(fs_path_to_id(path), next_free);
    free(full_name);
    return next_free;
}

void fs_write_in_file(char path[], char *data) {
    u_int32_t id_to_set = fs_path_to_id(path);
    i_write_in_file(id_to_set, data);
}

// IM FAST AS FUCK BOIII
u_int32_t fs_get_file_size(char path[]) {
    u_int32_t file_id = fs_path_to_id(path);
    // TODO : IF NOT A FILE
    u_int32_t buffer[SECTOR_SIZE];
    read_from_disk(file_id, buffer);
    return buffer[1 + MAX_SIZE_NAME + 1];
}

void *fs_declare_read_array(char path[]) {
    return calloc(fs_get_file_size(path) + 1, sizeof(char));
}

// How to declare data : char *data = fs_declare_read_array(path);
// How to free data    : free(data);
void fs_read_file(char path[], char *data) {
    u_int32_t file_size = fs_get_file_size(path);
    int data_index = 0;
    int sector = fs_path_to_id(path);
    u_int32_t buffer[SECTOR_SIZE];
    read_from_disk(sector, buffer);
    sector = buffer[SECTOR_SIZE-1];
    while (buffer[SECTOR_SIZE-1] != 0) {
        read_from_disk(sector, buffer);
        for (int i = MAX_SIZE_NAME + 1; i < SECTOR_SIZE - 1; i++) {
            data[data_index] = buffer[i];
            data_index++;
        }
        sector = buffer[SECTOR_SIZE-1];
    }
    data_index++;
    data[data_index] = '\0';
}

int main(int argc, char **argv) {
    init_fs();

    FILE *fptr;
    if ((fptr = fopen("HDD.bin","wb")) == NULL){
       printf("Error! opening file");

       // Program exits if the file pointer returns NULL.
       exit(1);
    }

    fs_make_dir("/", "test");
    fs_make_dir("/test", "test2");
    fs_make_file("/test/test2", "FILE");
    fs_make_file("/test/test2", "FILE");
    fs_write_in_file("/test/test2/FILE", "BITEBITEBITEBITEBITEBITEBITEBITEBITEBITEBITEBITEBITEBITEBITEBITEBITEBITEBITEBITEBITEBITEBITEBITEBITEBITEBITEBITEBITEBITEBITEBITEBITEBITEBITEBITEBITEBITEBITEBITEBITEBITEBITEBITEBITEBITEBITEBITEBITEBITEBITEBITEBITEBITE");
    printf("Le fichier %s fait %i characteres de long !\n", "/test/test2/FILE", fs_get_file_size("/test/test2/FILE"));
    char *file = fs_declare_read_array("/test/test2/FILE");
    fs_read_file("/test/test2/FILE", file);
    printf("Le fichier %s contient \"%s\"\n", "/test/test2/FILE", file);
    free(file);

    for (int i = 0; i < 8; i++) {
        i_print_sector(i);
    }

    int max_to_write = 8;
    for (int i = 0; i < max_to_write; i++) {
        u_int32_t buffer[SECTOR_SIZE];
        read_from_disk(i, buffer);
        fwrite(buffer, sizeof(u_int32_t), SECTOR_SIZE, fptr);
    }
    
    return 0;
}
