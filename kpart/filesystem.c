#include <kernel/filesystem.h>
#include <kernel/ramdisk.h>
#include <driver/ata.h>
#include <minilib.h>
#include <system.h>
#include <type.h>


#define MAX_SIZE_NAME  32
#define SECTOR_SIZE    128
#define FAST_SCAN      1 // use dichotomy to generate free map

#define I_FILE_H 0x1
#define I_FILE   0x10
#define I_DIR    0x100
#define I_DIRCNT 0x1000
#define I_USED   0x10000

uint8_t  *free_map;
uint32_t g_sector_count;


/**********************
 * PRIVATE FUNCTIONS *
**********************/

void i_generate_free_map();

void filesystem_init() {
    g_sector_count = ata_get_sectors_count();
    if (g_sector_count == 0) {
        sys_warning("Cannot use ATA disk, using ramdisk");
        g_sector_count = ramdisk_get_size();
    }
    
    free_map = calloc(g_sector_count * sizeof(uint8_t));
    i_generate_free_map();

    uint32_t root_sect[SECTOR_SIZE];
    ramdisk_read_sector(0, root_sect);

    if (!(root_sect[0] & 0x100)) {
        // TODO: init filesystem on empty disk
        // i_create_dir(0, "/");
        sys_fatal("Invalid root sector");
    }
}

void i_generate_free_map() {
    uint32_t buffer[SECTOR_SIZE];
    if (!FAST_SCAN) {    
        for (uint32_t i = 0; i < g_sector_count; i++) {
            ramdisk_read_sector(i, buffer);
            if (buffer[0] & I_USED) {
                free_map[i] = 1;
            } else {
                free_map[i] = 0;
            }
        }
        return;
    }
    // use dichotomy to find the last used sector
    uint32_t min = 0;
    uint32_t max = g_sector_count;
    uint32_t mid = 0;
    while (min < max) {
        mid = (min + max) / 2;
        ramdisk_read_sector(mid, buffer);
        if (buffer[0] & I_USED) {
            min = mid + 1;
        } else {
            max = mid;
        }
    }
    for (uint32_t i = 0; i < min; i++) {
        free_map[i] = 1;
    }
}

void i_declare_used(uint32_t sector) {
    if (g_sector_count < sector) {
        sys_error("Sector out of range");
        return;
    }
    free_map[sector] = 1;
}

void i_declare_free(uint32_t sector) {
    if (g_sector_count < sector) {
        sys_error("Sector out of range");
        return;
    }
    free_map[sector] = 0;
}

uint32_t i_next_free() {
    for (uint32_t i = 0; i < g_sector_count; i++) {
        if (free_map[i] != 0) continue;
        return i;
    }
    sys_fatal("No free sector");
    return 0;
}

char *i_build_path(char *path, char *name) {
    int len = 2;
    for (int i = 0; path[i] != '\0'; i++) len++;
    for (int i = 0; name[i] != '\0'; i++) len++;

    char *result = calloc(len * sizeof(char));

    str_cpy(result, path);
    if (path[str_len(path) - 1] != '/') {
        result[str_len(path)] = '/';
    }
    str_cat(result, name);

    return result;
}

void i_create_dir(uint32_t sector, char *name) {
    uint32_t buffer[SECTOR_SIZE];
    ramdisk_read_sector(sector, buffer);
    if (buffer[0] & I_USED) {
        sys_error("Sector already used");
        return;
    }
    if (str_len(name) > MAX_SIZE_NAME) {
        sys_error("Name too long");
        return;
    }
    for (int i = 0; i < SECTOR_SIZE; i++) {
        buffer[i] = 0;
    }
    buffer[0] = I_DIR | I_USED;
    for (int i = 0; i < str_len(name); i++) {
        buffer[1 + i] = name[i];
    }
    i_declare_used(sector);
    ramdisk_write_sector(sector, buffer);
}

void i_create_dir_continue(uint32_t sector) {
    uint32_t buffer[SECTOR_SIZE];
    ramdisk_read_sector(sector, buffer);
    if (buffer[0] & I_USED) {
        sys_error("Sector already used");
        return;
    }
    for (int i = 0; i < SECTOR_SIZE; i++) {
        buffer[i] = 0;
    }
    buffer[0] = I_DIRCNT | I_USED;
    i_declare_used(sector);
    ramdisk_write_sector(sector, buffer);
}

void dir_continue(uint32_t sector) {
    uint32_t buffer[SECTOR_SIZE];
    ramdisk_read_sector(sector, buffer);
    if (!(buffer[0] & I_USED)) {
        sys_error("Sector not used");
        return;
    }
    if (!(buffer[0] & (I_DIR | I_DIRCNT))) {
        sys_error("The sector isn't a directory");
        return;
    }
    uint32_t next_sector = i_next_free();
    buffer[SECTOR_SIZE-1] = next_sector;
    ramdisk_write_sector(sector, buffer);
    i_declare_used(next_sector);

    i_create_dir_continue(next_sector);
}

void i_add_item_to_dir(uint32_t dir_sector, uint32_t item_sector) {
    uint32_t buffer[SECTOR_SIZE];
    ramdisk_read_sector(dir_sector, buffer);
    if (!(buffer[0] & I_USED)) {
        sys_error("sector not used");
        return;
    }
    if (!(buffer[0] & (I_DIR | I_DIRCNT))) {
        sys_error("The sector isn't a directory");
        return;
    }
    for (int i = 1 + MAX_SIZE_NAME; i < SECTOR_SIZE-1; i++) {
        if (buffer[i] == 0) {
            buffer[i] = item_sector;
            ramdisk_write_sector(dir_sector, buffer);
            return;
        }
    }
    if (buffer[SECTOR_SIZE-1] == 0) {
        uint32_t next_sector = i_next_free();
        buffer[SECTOR_SIZE-1] = next_sector;
        ramdisk_write_sector(dir_sector, buffer);
        i_declare_used(next_sector);
        i_create_dir_continue(next_sector);
        i_add_item_to_dir(next_sector, item_sector);
    } else {
        i_add_item_to_dir(buffer[SECTOR_SIZE-1], item_sector);
    }
}

void i_remove_item_from_dir(uint32_t dir_sector, uint32_t item_sector) {
    uint32_t buffer[SECTOR_SIZE];
    ramdisk_read_sector(dir_sector, buffer);
    if (!(buffer[0] & I_USED)) {
        sys_error("Sector not used");
        return;
    }
    if (!(buffer[0] & (I_DIR | I_DIRCNT))) {
        sys_error("The sector isn't a directory");
        return;
    }
    for (int i = 1 + MAX_SIZE_NAME; i < SECTOR_SIZE-1; i++) {
        if (buffer[i] == item_sector) {
            buffer[i] = 0;
            ramdisk_write_sector(dir_sector, buffer);
        }
    }
    if (buffer[SECTOR_SIZE-1] != 0) {
        i_remove_item_from_dir(buffer[SECTOR_SIZE-1], item_sector);
    }
    // TODO : remove empty directory continues
}

int i_get_dir_size(uint32_t sector) {
    uint32_t buffer[SECTOR_SIZE];
    ramdisk_read_sector(sector, buffer);
    if (!(buffer[0] & I_USED)) {
        sys_error("Sector not used");
        return 0;
    }
    if (!(buffer[0] & (I_DIR | I_DIRCNT))) {
        sys_error("The sector isn't a directory");
        return 0;
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

void i_get_dir_content(uint32_t sector, uint32_t *ids, int index) {
    uint32_t buffer[SECTOR_SIZE];
    ramdisk_read_sector(sector, buffer);
    if (!(buffer[0] & I_USED)) {
        sys_error("Sector not used");
        return;
    }
    if (!(buffer[0] & (I_DIR | I_DIRCNT))) {
        sys_error("The sector isn't a directory");
        return;
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

void i_create_file_index(uint32_t sector, char *name) {
    uint32_t buffer[SECTOR_SIZE];
    ramdisk_read_sector(sector, buffer);
    if (buffer[0] & I_USED) {
        sys_error("Sector is already used");
        return;
    }
    if (str_len(name) > MAX_SIZE_NAME) {
        sys_error("Name is too long");
        return;
    }
    for (int i = 0; i < SECTOR_SIZE; i++) {
        buffer[i] = 0;
    }
    buffer[0] = I_FILE_H | I_USED;
    for (int i = 0; i < str_len(name); i++) {
        buffer[1 + i] = name[i];
    }
    i_declare_used(sector);
    ramdisk_write_sector(sector, buffer);
}

void i_write_in_file(uint32_t sector, uint8_t *data, uint32_t size) {
    uint32_t buffer[SECTOR_SIZE];
    ramdisk_read_sector(sector, buffer);
    if (!(buffer[0] & I_USED)) {
        sys_error("Sector not used");
        return;
    }
    if (!(buffer[0] & I_FILE_H)) {
        sys_error("The sector isn't a file header");
        return;
    }
    uint32_t next_sector = i_next_free();
    buffer[SECTOR_SIZE-1] = next_sector;
    buffer[MAX_SIZE_NAME + 2] = size;
    ramdisk_write_sector(sector, buffer);
    i_declare_used(next_sector);

    uint32_t *compressed_data = malloc(sizeof(uint32_t) * size);
    for (uint32_t i = 0; i < size; i++) {
        compressed_data[i] = data[i];
    }

    uint32_t sector_i, data_i;
    uint32_t current_sector = next_sector;
    data_i = 0;

    while (size > data_i) {
        buffer[0] = I_FILE | I_USED;
        for (sector_i = 1; sector_i < SECTOR_SIZE - 1; sector_i++) {
            if (size < data_i) break;
            buffer[sector_i] = compressed_data[data_i];
            data_i++;
        }
        if (size > data_i) {
            next_sector = i_next_free();
            buffer[SECTOR_SIZE - 1] = next_sector;
            i_declare_used(next_sector);
        } else {
            buffer[SECTOR_SIZE - 1] = 0;
        }
        ramdisk_write_sector(current_sector, buffer);
        if (size < data_i) break;
        current_sector = next_sector;
    }

    free(compressed_data);
}

uint32_t i_path_to_id(char *path, char *current_path, uint32_t sector) {
    // copy the current path
    char *current_path_copy = malloc(sizeof(char) * str_len(current_path) + 1);
    str_cpy(current_path_copy, current_path);

    // read the current sector
    uint32_t buffer[SECTOR_SIZE];
    ramdisk_read_sector(sector, buffer);

    // TODO: security check

    // get the name of the current sector
    char *name = malloc(MAX_SIZE_NAME + 1 * sizeof(char));
    for (int i = 0; i < MAX_SIZE_NAME; i++) {
        name[i] = (char) buffer[1 + i];
    }

    // add the name to the current path
    str_cat(current_path, name);
    free(name);
    
    if (current_path[str_len(current_path)-1] != '/') {
        str_cat(current_path, "/");
    }

    // if the current path is the path we are looking for, return the sector
    if (str_cmp(current_path, path) == 0) {
        free(current_path_copy);
        return sector;
    }

    // if current path is a prefix of the path we are looking for, go deeper
    if (str_ncmp(current_path, path, str_len(current_path)) == 0) {
        for (int i = MAX_SIZE_NAME + 1; i < SECTOR_SIZE-1; i++) {
            if (buffer[i] == 0) continue;
            
            uint32_t result = i_path_to_id(path, current_path, buffer[i]);
            if (result == 0) continue;

            free(current_path_copy);
            return result;
        }
    }

    // if we are here, the path is not found
    str_cpy(current_path, current_path_copy);
    free(current_path_copy);
    return 0;
}

void i_delete_dir(uint32_t id) {
    (void *)id;
}

void i_delete_file(uint32_t id) {
    (void *)id;
}

/*********************
 * PUBLIC FUNCTIONS *
*********************/

uint32_t fs_path_to_id(char *path) {
    char *edited_path = i_build_path(path, "");
    char *current_path = calloc(str_len(edited_path) + MAX_SIZE_NAME + 2);

    uint32_t exit_val = i_path_to_id(edited_path, current_path, 0);

    if (exit_val == 0 && str_cmp(edited_path, "/") != 0) {
        sys_warning("Path not found");
    }

    free(current_path);
    free(edited_path);
    return exit_val;
}

int fs_does_path_exists(char *path) {

    char *edited_path = i_build_path(path, "");
    char *current_path = calloc(str_len(edited_path) + MAX_SIZE_NAME + 2);


    uint32_t exit_val = i_path_to_id(edited_path, current_path, 0);

    if (str_cmp(edited_path, "/") == 0) exit_val = 1;

    free(current_path);
    free(edited_path);

    return exit_val != 0;
}

uint32_t fs_make_dir(char *path, char *name) {
    char *full_name = i_build_path(path, name);
    // TODO : check if there is a directory in path
    if (fs_does_path_exists(full_name)) {
        sys_error("Directory already exists");
        return -1;
    }
    uint32_t next_free = i_next_free();
    i_create_dir(next_free, name);
    i_add_item_to_dir(fs_path_to_id(path), next_free);
    free(full_name);
    return next_free;
}

uint32_t fs_make_file(char *path, char *name) {
    char *full_name = i_build_path(path, name);

    if (fs_does_path_exists(full_name)) {
        sys_error("File already exists");
        return -1;
    }

    uint32_t next_free = i_next_free();
    i_create_file_index(next_free, name);
    i_add_item_to_dir(fs_path_to_id(path), next_free);

    free(full_name);
    return next_free;
}

void fs_write_in_file(char *path, uint8_t *data, uint32_t size) {
    uint32_t id_to_set = fs_path_to_id(path);
    i_write_in_file(id_to_set, data, size);
}

uint32_t fs_get_file_size(char *path) {
    uint32_t file_id = fs_path_to_id(path);
    // TODO: security check
    uint32_t buffer[SECTOR_SIZE];
    ramdisk_read_sector(file_id, buffer);
    return buffer[MAX_SIZE_NAME + 2];
}

void *fs_declare_read_array(char *path) {
    return calloc(fs_get_file_size(path) + 1);
}

// How to declare data : char *data = fs_declare_read_array(path);
// How to free data    : free(data);
void fs_read_file(char *path, char *data) {
    uint32_t file_size = fs_get_file_size(path);
    uint32_t data_index = 0;
    int sector = fs_path_to_id(path);
    uint32_t buffer[SECTOR_SIZE];
    ramdisk_read_sector(sector, buffer);
    sector = buffer[SECTOR_SIZE - 1];
    while (buffer[SECTOR_SIZE-1] != 0) {
        ramdisk_read_sector(sector, buffer);
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
    uint32_t dir_id = fs_path_to_id(path);
    return i_get_dir_size(dir_id);
}

void fs_get_dir_content(char *path, uint32_t *ids) {
    uint32_t dir_id = fs_path_to_id(path);
    i_get_dir_content(dir_id, ids, 0);
}

void fs_get_element_name(uint32_t sector, char *name) {
    uint32_t buffer[SECTOR_SIZE];
    ramdisk_read_sector(sector, buffer);
    if (!(buffer[0] & I_USED)) {
        sys_error("Sector not used");
        return;
    }
    if (!(buffer[0] & (I_DIR | I_FILE_H))) {
        sys_error("The sector isn't a directory or a file\n");
        return;
    }
    for (int i = 0; i < MAX_SIZE_NAME; i++) {
        name[i] = buffer[i+1];
    }
}

int fs_get_sector_type(uint32_t sector_id) {
    uint32_t buffer[SECTOR_SIZE];
    ramdisk_read_sector(sector_id, buffer);

    if (!(buffer[0] & I_USED))  return 0; // sector empty
    if (buffer[0] & I_FILE)     return 1; // file content
    if (buffer[0] & I_FILE_H)   return 2; // file index
    if (buffer[0] & I_DIR)      return 3; // dir
    if (buffer[0] & I_DIRCNT)   return 4; // dir continue
    return 0;                             // sector empty
}

uint32_t fs_get_used_sectors() {
    uint32_t used_sectors = 0;
    for (uint32_t i = 0; i < g_sector_count; i++) {
        used_sectors += (free_map[i] == 1);
    }
    return used_sectors;
}

uint32_t fs_get_sector_count() {
    return g_sector_count;
}

int fs_delete_file(char *path) {
    uint32_t file_id = fs_path_to_id(path);
    if (file_id == 0) {
        return -1;
    }
    i_delete_file(file_id);
    return 0;
}

int fs_delete_dir(char *path) {
    uint32_t folder_id = fs_path_to_id(path);
    if (folder_id == 0) {
        return -1;
    }
    i_delete_dir(folder_id);
    return 0;
}

