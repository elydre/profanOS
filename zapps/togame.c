#include <stdint.h>
#define SC_H 72
#define SC_B 80
#define SC_E 01

#define Y_MAX 24
#define X_MAX 80

#define O_MAX 5

int start(int addr, int arg) {
    int (*get_func)(int id) = (int (*)(int)) addr;
    void (*clear_screen)() = (void (*)()) get_func(46);
    void (*ms_sleep)(int) = (void (*)(int)) get_func(44);
    void (*ckprint_at)(char *str, int x, int y, char color) = (void (*)(char *, int, int, char)) get_func(49);
    int (*get_last_scancode)() = (int (*)()) get_func(58);
    void (*sleep)(int) = (void (*)(int)) get_func(43);
    void (*int_to_ascii)(int n, char str[]) = (void (*)(int, char *)) get_func(23);
    void (*append)(char *str, char c) = (void (*)(char *, char)) get_func(29);
    int (*rand)() = (int (*)()) get_func(60);

    clear_screen();
    int val = 0;
    int last_sc = 0;
    int ox_s[O_MAX], oy_s[O_MAX];
    for (int i = 0; i < O_MAX; i++) {
        ox_s[i] = -1;
        oy_s[i] = -1;
    }
    int iter = 0;
    int nex_o = 0;
    int lost = 0;
    int to_wait = 40;
    char point[13];
    for (int y = 0; y < Y_MAX; y++) {
        for (int x = 0; x < X_MAX; x++) {
            ckprint_at(" ", x, y, 0x60);
        }
    }
    while (1) {
        if (!(iter % 20)) {
            ox_s[nex_o] = 80;
            oy_s[nex_o] = (rand() + oy_s[nex_o]) % Y_MAX;
            nex_o++;
            if (nex_o == O_MAX) nex_o = 0;
        }

        for (int i = 0; i < O_MAX; i++) {
            if (ox_s[i] == -1) continue;
            for (int x = ox_s[i] - 1; x < ox_s[i] + 3; x++) {
                for (int y = oy_s[i] - 1; y < oy_s[i] + 3; y++) {
                    if (y > Y_MAX - 1 || x >= X_MAX || x < 0) continue;
                    ckprint_at(" ", x, y, 0x60);
                }
            }
        }

        for (int i = 0; i < O_MAX; i++) {
            if (ox_s[i] == -1) continue;
            ox_s[i]--;
        }

        ckprint_at(" ", 10, val, 0x60);

        last_sc = get_last_scancode();
        if (last_sc == SC_B && val < Y_MAX - 1) val++;
        if (last_sc == SC_H && val > 0) val--;
        if (last_sc == SC_E) {
            clear_screen();
            return 0;
        }


        for (int i = 0; i < O_MAX; i++) {
            if (ox_s[i] == -1) continue;
            for (int x = ox_s[i]; x < ox_s[i] + 3; x++) {
                for (int y = oy_s[i]; y < oy_s[i] + 3; y++) {
                    if (y > Y_MAX - 1 || x >= X_MAX) continue;
                    ckprint_at(" ", x, y, 0x30);
                    if (val == y && x == 10) lost++;
                }
            }
        }
             
        if (lost == 0) ckprint_at("O", 10, val, 0x51);
        
        else {
            lost++;
            ckprint_at("X", 10, val, 0x41);
        }

        if (lost > 3) {
            ckprint_at(":( you lost", 0, 0, 0x60);
            sleep(5);
            clear_screen();
            break;
        }

        if (to_wait > 10) to_wait = 40 - (iter / 50);

        int_to_ascii(iter / 10, point);
        append(point, 'p');
        append(point, 't');
        append(point, 's');
        ckprint_at(point, 0, Y_MAX, 0x0f);
        ms_sleep(to_wait);
        iter++;
    }
    return arg;
}
