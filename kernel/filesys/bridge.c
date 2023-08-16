#include <kernel/butterfly.h>
#include <minilib.h>
#include <type.h>


uint32_t fs_path_to_id(char *path) {
    UNUSED(path);
    return 0;
}

int fs_does_path_exists(char *path) {
    UNUSED(path);
    return 0;
}

uint32_t fs_make_dir(char *path, char *name) {
    UNUSED(path);
    UNUSED(name);
    return 0;
}

uint32_t fs_make_file(char *path, char *name) {
    UNUSED(path);
    UNUSED(name);
    return 0;
}

void *fs_declare_read_array(char *path) {
    UNUSED(path);
    return NULL;
}

void fs_write_in_file(char *path, uint8_t *data, uint32_t size) {
    UNUSED(path);
    UNUSED(data);
    UNUSED(size);
}

void fs_read_file(char *path, char *data) {
    UNUSED(path);
    UNUSED(data);
}

uint32_t fs_get_file_size(char *path) {
    UNUSED(path);
    return 0;
}

int fs_get_dir_size(char *path) {
    UNUSED(path);
    return 0;
}

void fs_get_dir_content(char *path, uint32_t *ids) {
    UNUSED(path);
    UNUSED(ids);
}

void fs_get_element_name(uint32_t sector, char *name) {
    UNUSED(sector);
    UNUSED(name);
}

int fs_get_sector_type(uint32_t sector_id) {
    UNUSED(sector_id);
    return 0;
}

uint32_t fs_get_used_sectors() {
    return 0;
}

uint32_t fs_get_sector_count() {
    return 0;
}

int fs_delete_file(char *path) {
    UNUSED(path);
    return 0;
}

int fs_delete_dir(char *path) {
    UNUSED(path);
    return 0;
}
