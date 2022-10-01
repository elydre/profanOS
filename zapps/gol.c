#include "addf.h"

void printl(int addr, int **plateau);
void next_step(int addr, int **plateau);
int size = 20;

int main(int addr, int arg) {
    INIT_AF(addr);
    AF_calloc();
    AF_free();
    AF_kb_get_scancode();

    // init du plateau
    int **plateau = calloc(size * sizeof(int *));
    for (int i = 0; i < size; i++)
        plateau[i] = calloc(size * sizeof(int));

    plateau[2][6] = 1;
    plateau[3][6] = 1;
    plateau[4][6] = 1;

    while (kb_get_scancode() != 1) {
        printl(addr, plateau);
        next_step(addr, plateau);
    }

    // fin
    for (int i = 0; i < size; i++) free(plateau[i]);
    free(plateau);
    return arg;
}

void printl(int addr, int **plateau) {
    INIT_AF(addr);
    AF_calloc();
    AF_free();
    AF_ckprint_at();

    char * ligne = calloc((size+size-1)*sizeof(char));
    int offset;
    for (int i = 0; i < size; i++) {
        offset = 0;
        for (int j = 0; j < size; j++) {
            ligne[offset] = plateau[i][j] ? 'X' : '.';
            offset++;
            ligne[offset] = ' ';
            offset++;
        }
        ckprint_at(ligne, 0, i, 0x0F);
    }
    free(ligne);
}

int get_value(int i, int j, int **plateau, int i2, int j2) {
    if (i == i2 && j == j2) return 0;
    // renvoie la valeur d'une case, si en dehors du plateau renvoie 0
    if (i < 0 || i >= size || j < 0 || j >= size) return 0;
    return plateau[i][j];
}

int next_value(int valeur, int i, int j, int **plateau) {

    // FONCTIONNEMENT DU JEU
    // survie si 2 ou 3 voisine
    // sinon meure
    // si case vide avec 3 voisine -> naissance

    int total = 0;
    for (int off_i = -1; off_i <= 1; off_i++) {
        for (int off_j = -1; off_j <= 1; off_j++) {
            total = total + get_value(i+off_i, j + off_j, plateau, i, j);
        }
    }

    switch (total) {
        case 0: return 0;
        case 1: return 0;
        case 2: return 1 & valeur;
        case 3: return 1;
        case 4: return 0; 
        case 5: return 0;
        case 6: return 0;
        case 7: return 0;
        case 8: return 0;
        case 9: return 0;
        default: return 0;
    }
}

void next_step(int addr, int **plateau) {
    INIT_AF(addr);
    AF_calloc();
    AF_free();
    AF_ms_sleep();

    // plateau temp
    int **plateau_temp = calloc(size * sizeof(int *));
    for (int i = 0; i < size; i++)
        plateau_temp[i] = calloc(size * sizeof(int));

    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            plateau_temp[i][j] = next_value(plateau[i][j], i, j, plateau);
        }
    }

    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            plateau[i][j] = plateau_temp[i][j];
        }
    }
    
    for (int i = 0; i < size; i++) free(plateau_temp[i]);
    free(plateau_temp);

    ms_sleep(250);
}
