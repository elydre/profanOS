#ifndef FILESYS_H
#define FILESYS_H
#include <stdint.h>

uint32_t i_next_free(uint32_t rec);
uint32_t i_creer_dossier(char nom[]);
uint32_t i_creer_index_de_fichier(char nom[]);

#endif
