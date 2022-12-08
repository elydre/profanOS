#include <syscall.h>
#include <string.h>

#define SC_H 72
#define SC_B 80
#define SC_E 01

#define Y_MAX 24
#define X_MAX 80

#define O_MAX 5

int main(int argc, char **argv) {

    c_clear_screen();
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
            c_ckprint_at(" ", x, y, 0);
        }
    }
    while (1) {
        if (!(iter % 20)) {
            ox_s[nex_o] = 80;
            oy_s[nex_o] = (c_rand() + oy_s[nex_o]) % Y_MAX;
            nex_o++;
            if (nex_o == O_MAX) nex_o = 0;
        }

        for (int i = 0; i < O_MAX; i++) {
            if (ox_s[i] == -1) continue;
            for (int x = ox_s[i] - 1; x < ox_s[i] + 3; x++) {
                for (int y = oy_s[i] - 1; y < oy_s[i] + 3; y++) {
                    if (y > Y_MAX - 1 || x >= X_MAX || x < 0) continue;
                    c_ckprint_at(" ", x, y, 0);
                }
            }
        }

        for (int i = 0; i < O_MAX; i++) {
            if (ox_s[i] == -1) continue;
            ox_s[i]--;
        }

        c_ckprint_at(" ", 10, val, 0);

        last_sc = c_kb_get_scancode();
        if (last_sc == SC_B && val < Y_MAX - 1) val++;
        if (last_sc == SC_H && val > 0) val--;
        if (last_sc == SC_E) {
            c_clear_screen();
            return 0;
        }


        for (int i = 0; i < O_MAX; i++) {
            if (ox_s[i] == -1) continue;
            for (int x = ox_s[i]; x < ox_s[i] + 3; x++) {
                for (int y = oy_s[i]; y < oy_s[i] + 3; y++) {
                    if (y > Y_MAX - 1 || x >= X_MAX) continue;
                    c_ckprint_at(" ", x, y, 0x30);
                    if (val == y && x == 10) lost++;
                }
            }
        }
             
        if (lost == 0) c_ckprint_at("O", 10, val, 0x51);
        
        else {
            lost++;
            c_ckprint_at("X", 10, val, 0x41);
        }

        if (lost > 3) {
            c_ckprint_at(":( you lost", 0, 0, 0x0f);
            c_ms_sleep(5000);
            c_clear_screen();
            break;
        }

        if (to_wait > 10) to_wait = 40 - (iter / 50);

        int_to_ascii(iter / 10, point);
        str_append(point, 'p');
        str_append(point, 't');
        str_append(point, 's');
        c_ckprint_at(point, 0, Y_MAX, 0x0f);
        c_ms_sleep(to_wait);
        iter++;
    }
    return 0;
}
