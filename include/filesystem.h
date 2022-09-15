#ifndef FILESYS_H
#define FILESYS_H
#include <stdint.h>

typedef struct string_20 {
    char name[20];
} string_20;

uint32_t i_next_free(uint32_t rec);
uint32_t i_creer_dossier(char nom[]);
uint32_t i_creer_index_de_fichier(char nom[]);
void i_set_data_to_file(char data[], uint32_t data_size, uint32_t file_id);
void i_add_item_to_dir(uint32_t file_id, uint32_t folder_id);
void i_get_dir_content(uint32_t id, string_20 list_name[], int liste_id[]);

#endif
