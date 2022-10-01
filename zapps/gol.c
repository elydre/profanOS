#include "addf.h"

void printl(int addr, int **plateau, int force_print);
void next_step(int addr, int **plateau);
void edition_state(int addr, int **plateau);
int size = 20;

int main(int addr, int arg) {
    INIT_AF(addr);

    AF_kb_get_scancode();
    AF_cursor_blink();
    AF_fskprint();
    AF_calloc();
    AF_free();
    AF_ckprint_at();

    cursor_blink(1);

    // init du plateau
    int **plateau = calloc(size * sizeof(int *));
    for (int i = 0; i < size; i++)
        plateau[i] = calloc(size * sizeof(int));

    plateau[2][6] = 1;
    plateau[3][6] = 1;
    plateau[4][6] = 1;

    while (kb_get_scancode() != 1) {
        printl(addr, plateau, 2);
        ckprint_at("Commandes :", 0, size, 0x0F);
        ckprint_at("ECHAP : quitter", 0, size+1, 0x0F);
        ckprint_at("E     : mode edition", 0, size+2, 0x0F);
        next_step(addr, plateau);
        if (kb_get_scancode() == 18) {
            edition_state(addr, plateau);
        }
    }

    // fin
    for (int i = 0; i < size; i++) free(plateau[i]);
    free(plateau);
    cursor_blink(0);
    fskprint("\n");
    return arg;
}

void edition_state(int addr, int **plateau) {
    INIT_AF(addr);
    AF_ckprint_at();
    AF_kb_get_scancode();
    AF_clear_screen();

    int curseur_x = 0;
    int curseur_y = 0;
    int ancienne_valeur = plateau[curseur_x][curseur_y];
    plateau[curseur_x][curseur_y] = 219;
    int last_scancode;
    int scancode = 0;

    clear_screen();

    while (kb_get_scancode() != 30) {
        printl(addr, plateau, 1);
        ckprint_at("COMMANDES MODE EDITION :", 0, size, 0x0F);
        ckprint_at("Q     : quitter", 0, size+1, 0x0F);
        ckprint_at("F     : effacer l'ecran", 0, size+2, 0x0F);
        last_scancode = scancode;
        scancode = kb_get_scancode();
        if (last_scancode != scancode) {
            if (scancode == 72) {
                if (curseur_x > 0) {
                    plateau[curseur_x][curseur_y] = ancienne_valeur;
                    curseur_x--;
                    ancienne_valeur = plateau[curseur_x][curseur_y];
                    plateau[curseur_x][curseur_y] = 219;
                }
            }
            else if (scancode == 75) {
                if (curseur_y > 0) {
                    plateau[curseur_x][curseur_y] = ancienne_valeur;
                    curseur_y--;
                    ancienne_valeur = plateau[curseur_x][curseur_y];
                    plateau[curseur_x][curseur_y] = 219;
                }
            }
            else if (scancode == 80) {
                if (curseur_x < size-1) {
                    plateau[curseur_x][curseur_y] = ancienne_valeur;
                    curseur_x++;
                    ancienne_valeur = plateau[curseur_x][curseur_y];
                    plateau[curseur_x][curseur_y] = 219;
                }
            }
            else if (scancode == 77) {
                if (curseur_y < size-1) {
                    plateau[curseur_x][curseur_y] = ancienne_valeur;
                    curseur_y++;
                    ancienne_valeur = plateau[curseur_x][curseur_y];
                    plateau[curseur_x][curseur_y] = 219;
                }
            }
            else if (scancode == 28) {
                ancienne_valeur = ancienne_valeur ? 0 : 1;
            }
            else if (scancode == 33) {
                for (int i = 0; i < size; i++) {
                    for (int j = 0; j < size; j++) {
                        plateau[i][j] = 0;
                    }
                }
            }
            else ckprint_at("      ", 0, size+3, 0x0F);
        }
    }
    
    plateau[curseur_x][curseur_y] = ancienne_valeur;
    ancienne_valeur = 0;

    clear_screen();
}

void printl(int addr, int **plateau, int force_print) {
    INIT_AF(addr);

    AF_ckprint_at();
    AF_calloc();
    AF_free();

    char * ligne = calloc((size+size-1)*sizeof(char));
    int offset;
    for (int i = 0; i < size; i++) {
        offset = 0;
        for (int j = 0; j < size; j++) {
            if (!force_print) ligne[offset] = plateau[i][j] ? 'X' : '.';
            else {
                if (plateau[i][j] == 1) ligne[offset] = 'X';
                else if (plateau[i][j] == 0) ligne[offset] = '.';
                else ligne[offset] = plateau[i][j];
            }
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
    
    AF_ms_sleep();
    AF_calloc();
    AF_free();

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
