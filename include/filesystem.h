#ifndef FILESYS_H
#define FILESYS_H
#include <stdint.h>

typedef struct string_20_t {
    char name[20];
} string_20_t;

void init_filesystem();

uint32_t get_used_sectors(uint32_t disk_size);
uint32_t is_disk_full(uint32_t disk_size);


uint32_t make_dir(char path[], char folder_name[]);
uint32_t make_file(char path[], char file_name[]);

void read_file(char path[], uint32_t data[]);
void write_in_file(char path[], uint32_t data[], uint32_t data_size);

uint32_t get_file_size(char path[]);
int get_folder_size(char path[]);

void * declare_read_array(char path[]);

int does_path_exists(char path[]);
int type_sector(uint32_t id_sector);
void get_dir_content(uint32_t id, string_20_t list_name[], uint32_t liste_id[]);
uint32_t path_to_id(char input_path[], int silence);

#endif
