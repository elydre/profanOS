/*     _             _
  ___ | | _   _   __| | _ __   ___
 / _ \| || | | | / _` || '__| / _ |
|  __/| || |_| || (_| || |   |  __/
 \___||_| \__, | \__,_||_|    \___|
          |___/
___________________________________

 - codé en : UTF-8
 - langage : c
 - GitHub  : github.com/elydre
 - Licence : GNU GPL v3
*/

#include <syscall.h>
#include <string.h>
#include <stdio.h>
#include <i_time.h>
#include <stdlib.h>
#include <i_iolib.h>

char get_piont(int num);
int get_user_choix(int ** grille);
void chute(int colonne, int ** grille);
void free_grille(int ** grille);
int IA_play(int ** grille);
int is_gagnant(int ** tab);

int main(int argc, char **argv) {
    int colonne;
    int tour = 0;

    int **grille = (int **) calloc(8, sizeof(int *));
    for (int i = 0; i < 8; i++) {
        grille[i] = (int *) calloc(8, sizeof(int));
    }

    c_clear_screen();
    while (1) {
        colonne = (tour) ? IA_play(grille) : get_user_choix(grille);

        if (colonne == -1) {
            c_clear_screen();
            free_grille(grille);
            return 0;
        }

        grille[colonne][0] = tour + 1;
        chute(colonne, grille);

        if (is_gagnant(grille) > 0)
            break;

        tour = !tour;
    }

    printf((char *) "\nWinner is %c\n\n", get_piont(tour + 1));
    free_grille(grille);
    return 0;
}

int IA_cp_gagnant(int joueur, int ** grille) {
    int ** grille_test = (int **) calloc(8, sizeof(int *));
    for (int i = 0; i < 8; i++) {
        grille_test[i] = (int *) calloc(8, sizeof(int));
    }
    for (int c = 0; c < 8; c++) {
        for (int l = 0; l < 8; l++) {
            for (int j = 0; j < 8; j++) {
                grille_test[l][j] = grille[l][j];
            }
        }
        grille_test[c][0] = joueur;
        for (int l = 0; l < 7; l++) {
            if (grille_test[c][l] > 0 && grille_test[c][l + 1] == 0) {
                grille_test[c][l + 1] = grille_test[c][l];
                grille_test[c][l] = 0;
            }
        }
        if (is_gagnant(grille_test) > 0) {
            free_grille(grille_test);
            return c;
        }
    }
    free_grille(grille_test);
    return -1;
}

int IA_play(int ** grille) {
    int val = IA_cp_gagnant(2, grille);
    if (val >= 0)
        return val;

    val = IA_cp_gagnant(1, grille);
    if (val >= 0)
        return val;

    while (1) {
        val = c_rand() % 8;
        if (val < 8 && val >= 0 && grille[val][0] == 0) {
            return val;
        }
    }
}

char get_piont(int num) {
    if (num == 1) return '@';
    if (num == 2) return 'X';
    if (num == 0) return ' ';
    return '?';
}

void print_grille(int ** grille) {
    c_ckprint_at((char *) "   1   2   3   4   5   6   7   8\n\n", 0, 0, 0x0F);

    for (int l = 0; l < 8; l++) {
        for (int c = 0; c < 8; c++) {
            printf((char *) " | %c", get_piont(grille[c][l]));
        }
        printf((char *) " |\n");
    }
    printf((char *) "\n");
}

int get_user_choix(int ** grille) {
    int inp;
    print_grille(grille);
    char buffer[3];

    while (1) {
        c_ckprint_at((char *) "ENTER YOUR CHOICE -> ", 0, 11, 0x0F);
        input(buffer, 3, 0x0F);
        inp = atoi(buffer) - 1;

        if (inp == -1 && buffer[0]) return -1;
        if (inp < 8 && inp >= 0 && grille[inp][0] == 0) return inp;
    }
}

void chute(int colonne, int ** grille) {
    for (int l = 0; l < 7; l++) {
        if (grille[colonne][l] > 0 && grille[colonne][l + 1] == 0) {
            grille[colonne][l + 1] = grille[colonne][l];
            grille[colonne][l] = 0;
            print_grille(grille);
            ms_sleep(100);
        }
    }
}

int check_eg(int v1, int v2, int v3, int v4) {
    if (v1 == v2 && v1 == v3 && v1 == v4) return v1;
    return 0;
}

int is_gagnant(int ** tab) {
    // ligne & colonnes

    for (int c = 0; c < 8; c++) {
        for (int l = 0; l < 5; l++) {
            if (check_eg(tab[c][l], tab[c][l + 1], tab[c][l + 2], tab[c][l + 3]) > 0) return 1;
            if (check_eg(tab[l][c], tab[l + 1][c], tab[l + 2][c], tab[l + 3][c]) > 0) return 2;
        }
    }

    // diagonales

    for (int c = 0; c < 5; c++) {
        for (int l = 0; l < 5; l++) {
            if (check_eg(tab[c][l], tab[c + 1][l + 1], tab[c + 2][l + 2], tab[c + 3][l + 3]) > 0) return 3;
            if (check_eg(tab[l +3 ][c], tab[l + 2][c + 1], tab[l + 1][c + 2], tab[l][c + 3]) > 0) return 4;
        }
    }
    return 0;
}

void free_grille(int ** grille) {
    for (int i = 0; i < 8; i++) {
        free(grille[i]);
    }
    free(grille);
}
