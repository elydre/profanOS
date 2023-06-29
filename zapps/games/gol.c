#include <syscall.h>
#include <stdlib.h>
#include <stdio.h>

#include <i_ocmlib.h> 
#include <i_iolib.h>
#include <i_time.h>

void printl(int **plateau, int curseur_x, int curseur_y);
void next_step(int **plateau);
void edition_state(int **plateau);
int size_x = 20;
int size_y = 40;
int wait = 250;

int main(int argc, char **argv) {

    ocm_clear();

    // init du plateau
    int **plateau = calloc(size_x, sizeof(int *));
    for (int i = 0; i < size_x; i++)
        plateau[i] = calloc(size_y, sizeof(int));

    plateau[2][6] = 1;
    plateau[3][6] = 1;
    plateau[4][6] = 1;

    int last_scancode, scancode = 0;

    while (scancode != 1) {
        last_scancode = scancode;
        scancode = c_kb_get_scancode();
        if (c_kb_get_scancode() == 18)
            edition_state(plateau);
        if (c_kb_get_scancode() == 25) {
            wait += 50;
            while (c_kb_get_scancode() == 25);
        }
        if (c_kb_get_scancode() == 39 && wait > 0) {
            wait -= 50;
            while (c_kb_get_scancode() == 39);
        }
        if (scancode != last_scancode) {
            ocm_print("Commandes :", 0, size_x+1, 0xFFFFFF, 0x000000);
            ocm_print("ECHAP : quitter", 0, size_x+2, 0xFFFFFF, 0x000000);
            ocm_print("E     : mode edition\n", 0, size_x+3, 0xFFFFFF, 0x000000);
            printf("P/M   : ms_sleep(%d); ", wait);
        };
        next_step(plateau);
        printl(plateau, -1, -1);
    }

    // fin
    for (int i = 0; i < size_x; i++) free(plateau[i]);
    free(plateau);
    ocm_clear();
    
    return 0;
}

void edition_state(int **plateau) {

    int curseur_x = 0;
    int curseur_y = 0;
    int last_scancode;
    int scancode = 0;

    ocm_clear();

    while (c_kb_get_scancode() != 30) {
        printl(plateau, curseur_x, curseur_y);
        ocm_print("COMMANDES MODE EDITION :", 0, size_x+1, 0xFFFFFF, 0x000000);
        ocm_print("Q     : quitter", 0, size_x+2, 0xFFFFFF, 0x000000);
        ocm_print("F     : effacer l'ecran", 0, size_x+3, 0xFFFFFF, 0x000000);
        last_scancode = scancode;
        scancode = c_kb_get_scancode();
        if (last_scancode == scancode) continue;
        if (scancode == 72) curseur_x -= curseur_x > 0;
        else if (scancode == 75) curseur_y -= curseur_y > 0;
        else if (scancode == 80) curseur_x += curseur_x < size_x - 1;
        else if (scancode == 77) curseur_y += curseur_y < size_y - 1;
        else if (scancode == 28 || scancode == 57)
            plateau[curseur_x][curseur_y] = !plateau[curseur_x][curseur_y];
        else if (scancode == 33) {
            for (int i = 0; i < size_x; i++)
                for (int j = 0; j < size_y; j++)
                    plateau[i][j] = 0;
        }
    }
    ocm_clear();
}

void printl(int **plateau, int curseur_x, int curseur_y) {

    char *ligne = calloc((size_y+size_y-1), sizeof(char));
    int offset;
    for (int i = 0; i < size_x; i++) {
        offset = 0;
        if (i == curseur_x) {
            for (int j = 0; j < curseur_y; j++) {
                ligne[offset] = plateau[i][j] ? 'X' : '.';
                ligne[offset + 1] = ' ';
                offset += 2;
            }
            ligne[offset] = '\0';
            ocm_print(ligne, 0, i, 0xFFFFFF, 0x000000);
            ligne[0] = plateau[i][curseur_y] ? 'X' : '.';
            ligne[1] = '\0';
            ocm_print(ligne, curseur_y * 2, i, 0x000000, 0xFFFFFF);
            offset = 0;
            for (int j = curseur_y + 1; j < size_y; j++) {
                ligne[offset] = plateau[i][j] ? 'X' : '.';
                ligne[offset + 1] = ' ';
                offset += 2;
            }
            ligne[offset] = '\0';
            ocm_print(ligne, (curseur_y+1)*2, i, 0xFFFFFF, 0x000000);
        } else {
            for (int j = 0; j < size_y; j++) {
                ligne[offset] = plateau[i][j] ? 'X' : '.';
                ligne[offset + 1] = ' ';
                offset += 2;
            }
            ligne[size_y*2-1] = '\0';
            ocm_print(ligne, 0, i, 0xFFFFFF, 0x000000);
        }
    }
    free(ligne);
}

int get_value(int i, int j, int **plateau, int i2, int j2) {
    if (i == i2 && j == j2) return 0;
    // renvoie la valeur d'une case, si en dehors du plateau renvoie 0
    if (i < 0 || i >= size_x || j < 0 || j >= size_y) return 0;
    return plateau[i][j];
}

int next_value(int valeur, int i, int j, int **plateau) {

    // FONCTIONNEMENT DU JEU
    // survie si 2 ou 3 voisine
    // sinon meure
    // si case vide avec 3 voisine -> naissance

    int total = 0;
    for (int off_i = -1; off_i <= 1; off_i++)
        for (int off_j = -1; off_j <= 1; off_j++)
            total = total + get_value(i+off_i, j + off_j, plateau, i, j);

    switch (total) {
        case 2: return 1 & valeur;
        case 3: return 1;
        default: return 0;
    }
}

void next_step(int **plateau) {

    // plateau temp
    int **plateau_temp = calloc(size_x, sizeof(int *));
    for (int i = 0; i < size_x; i++)
        plateau_temp[i] = calloc(size_y, sizeof(int));

    for (int i = 0; i < size_x; i++)
        for (int j = 0; j < size_y; j++)
            plateau_temp[i][j] = next_value(plateau[i][j], i, j, plateau);

    for (int i = 0; i < size_x; i++)
        for (int j = 0; j < size_y; j++)
            plateau[i][j] = plateau_temp[i][j];

    for (int i = 0; i < size_x; i++) free(plateau_temp[i]);
    free(plateau_temp);

    ms_sleep(wait);
}
