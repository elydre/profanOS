/*     _             _
  ___ | | _   _   __| | _ __   ___
 / _ \| || | | | / _` || '__| / _ |
|  __/| || |_| || (_| || |   |  __/
 \___||_| \__, | \__,_||_|    \___|
          |___/
___________________________________

 - codé en : UTF-8
 - langage : c++
 - GitHub  : github.com/elydre
 - Licence : GNU GPL v3
*/


int grille[8][8];
static int r = 0;

char get_piont(int num);
void print_grille(int addr);
void clear_all();
int get_user_choix(int addr);
void chute(int colonne, int addr);
int check_eg(int v1, int v2, int v3, int v4);
int is_gagnant(int tab[8][8]);
int randint();

class IA {
private:
    int grille_test[8][8];

    void vute(int colonne) {
        for (int l = 0; l < 7; l++) {
            if (grille_test[colonne][l] > 0 && grille_test[colonne][l + 1] == 0) {
                grille_test[colonne][l + 1] = grille_test[colonne][l];
                grille_test[colonne][l] = 0;
            }
        }
    }

    void push_grille() {
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                grille_test[i][j] = grille[i][j];
            }
        }
    }

    int cp_gagnant(int joueur) {
        for (int c = 0; c < 8; c++) {
            push_grille();
            grille_test[c][0] = joueur;
            vute(c);
            if (is_gagnant(grille_test) > 0) {
                return c;
            }
        }
        return -1;
    }

public:
    int play() {
        int val = cp_gagnant(2);
        if (val >= 0)
            return val;

        val = cp_gagnant(1);
        if (val >= 0)
            return val;

        while (1) {
            val = randint();
            if (val < 8 && val >= 0 && grille[val][0] == 0) {
                return val;
            }
        }
    }
};

int main (int arg) {

    int addr = (int) *(int *) 0x199990;

    int (*get_func)(int id) = (int (*)(int)) addr;
    void (*fskprint)(char * format, ...) = (void (*)(char*, ...)) get_func(38);
    void (*cursor_blink)(int on) = (void (*)(int)) get_func(76);
    void (*clear_screen)() = (void (*)()) get_func(46);
    int (*time_gen_unix)() = (int (*)()) get_func(42);


    r = time_gen_unix();

    int colonne;
    bool tour = 0;
    IA joueur2;

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            grille[i][j] = 0;
        }
    }

    clear_screen();
    while (1) {
        colonne = (tour) ? joueur2.play() : get_user_choix(addr);

        if (colonne == -1) {
            clear_screen();
            return 0;
        }

        grille[colonne][0] = tour + 1;
        chute(colonne, addr);

        if (is_gagnant(grille) > 0)
            break;

        tour = !tour;
    }

    fskprint((char *) "\nWinner is %c\n\n", get_piont(tour + 1));
    cursor_blink(0);
    return arg;
}

char get_piont(int num) {
    if (num == 1) return '@';
    if (num == 2) return 'X';
    if (num == 0) return ' ';
    return '?';
}

void print_grille(int addr) {
    int (*get_func)(int id) = (int (*)(int)) addr;
    void (*ckprint_at)(char *str, int x, int y, char color) = (void (*)(char *, int, int, char)) get_func(49);
    void (*fskprint)(char * format, ...) = (void (*)(char*, ...)) get_func(38);

    ckprint_at((char *) "   1   2   3   4   5   6   7   8\n\n", 0, 0, 0x0F);

    for (int l = 0; l < 8; l++) {
        for (int c = 0; c < 8; c++) {
            fskprint((char *) " | %c", get_piont(grille[c][l]));
        }
        fskprint((char *) " |\n");
    }
    fskprint((char *) "\n");
}

int get_user_choix(int addr) {
    int (*get_func)(int id) = (int (*)(int)) addr;
    void (*input)(char out_buffer[], int size, char color) = (void (*)(char *, int, char)) get_func(41);
    void (*ckprint_at)(char *str, int x, int y, char color) = (void (*)(char *, int, int, char)) get_func(49);
    int (*ascii_to_int)(char str[]) = (int (*)(char[])) get_func(25);
    void (*cursor_blink)(int on) = (void (*)(int)) get_func(76);

    int inp;

    print_grille(addr);

    char buffer[3];
    while (1) {

        ckprint_at((char *) "ENTER YOUR CHOICE -> ", 0, 11, 0x0F);
        cursor_blink(0);
        input(buffer, 3, 0x0F);
        cursor_blink(1);
        inp = ascii_to_int(buffer) - 1;

        if (inp == -1 && buffer[0]) return -1;
        if (inp < 8 && inp >= 0 && grille[inp][0] == 0) return inp;
    }
}

void chute(int colonne, int addr) {
    int (*get_func)(int id) = (int (*)(int)) addr;
    void (*ms_sleep)(int) = (void (*)(int)) get_func(44);

    for (int l = 0; l < 7; l++) {
        if (grille[colonne][l] > 0 && grille[colonne][l + 1] == 0) {
            grille[colonne][l + 1] = grille[colonne][l];
            grille[colonne][l] = 0;
            print_grille(addr);
            ms_sleep(100);
        }
    }
}

int check_eg(int v1, int v2, int v3, int v4) {
    if (v1 == v2 && v1 == v3 && v1 == v4) return v1;
    return 0;
}

int is_gagnant(int tab[8][8]) {

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

int rand() {
    r = r * 0x343FD + 0x269EC3;
    return (r >> 16) & 0x7FFF;
}

int randint() {
    int r = 0;
    for (int i = 0; i < 8; i++) {
        r = r << 1;
        r = r | (rand() & 1);
    }
    return r;
}
