#ifndef FILESYS_H
#define FILESYS_H
#include <stdint.h>

typedef struct string_20_t {
    char name[20];
} string_20_t;

void init_filesystem();

int get_used_sectors(int disk_size);
int is_disk_full(int disk_size);

uint32_t make_dir(char path[], char folder_name[]);
uint32_t make_file(char path[], char file_name[]);
void write_in_file(char path[], uint32_t data[], uint32_t data_size);
uint32_t get_file_size(char path[]);
void read_file(char path[], uint32_t data[]);
void *declare_read_array(char path[]);
int does_path_exists(char path[]);
void list_dir_content(char path[], string_20_t out_list[]);

#endif
