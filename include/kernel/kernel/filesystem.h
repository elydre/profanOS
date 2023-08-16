#ifndef FILESYS_H
#define FILESYS_H

#include <type.h>

int      filesys_init();

uint32_t fs_path_to_id(char *path);
int      fs_does_path_exists(char *path);

uint32_t fs_make_dir(char *path, char *name);
uint32_t fs_make_file(char *path, char *name);

void    *fs_declare_read_array(char *path);

void     fs_write_in_file(char *path, uint8_t *data, uint32_t size);
void     fs_read_file(char *path, char *data);

uint32_t fs_get_file_size(char *path);
int      fs_get_dir_size(char *path);
void     fs_get_dir_content(char *path, uint32_t *ids);

void     fs_get_element_name(uint32_t sector, char *name);
int      fs_get_sector_type(uint32_t sector_id);

uint32_t fs_get_used_sectors();
uint32_t fs_get_sector_count();

int      fs_delete_file(char *path);
int      fs_delete_dir(char *path);

#endif
