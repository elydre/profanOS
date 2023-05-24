#include <syscall.h>
#include <string.h>
#include <stdlib.h>

#include <i_ocmlib.h>
#include <i_time.h>

#define SC_H 72
#define SC_B 80
#define SC_E 01

#define Y_MAX 24
#define X_MAX 80

#define O_MAX 5

int main(int argc, char **argv) {
    ocm_clear();

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
            ocm_print(" ", x, y, 0x000000, 0x000000);
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
                    ocm_print(" ", x, y, 0x000000, 0x000000);
                }
            }
        }

        for (int i = 0; i < O_MAX; i++) {
            if (ox_s[i] == -1) continue;
            ox_s[i]--;
        }

        ocm_print(" ", 10, val, 0x000000, 0x000000);

        last_sc = c_kb_get_scancode();
        if (last_sc == SC_B && val < Y_MAX - 1) val++;
        if (last_sc == SC_H && val > 0) val--;
        if (last_sc == SC_E) {
            ocm_clear();
            return 0;
        }


        for (int i = 0; i < O_MAX; i++) {
            if (ox_s[i] == -1) continue;
            for (int x = ox_s[i]; x < ox_s[i] + 3; x++) {
                for (int y = oy_s[i]; y < oy_s[i] + 3; y++) {
                    if (y > Y_MAX - 1 || x >= X_MAX) continue;
                    ocm_print(" ", x, y, 0xFF0000, 0xFF0000);
                    if (val == y && x == 10) lost++;
                }
            }
        }
             
        if (lost == 0) ocm_print("O", 10, val, 0x00FF00, 0x000000);
        
        else {
            lost++;
            ocm_print("X", 10, val, 0x0000FF, 0x000000);
        }

        if (lost > 3) {
            ocm_print(":( you lost", 0, 0, 0xFFFF00, 0x000000);
            ms_sleep(5000);
            ocm_clear();
            break;
        }

        if (to_wait > 10) to_wait = 40 - (iter / 50);

        itoa(iter / 10, point, 10);
        strcat(point, "pts");
        ocm_print(point, 0, Y_MAX, 0xFFFFFF, 0x000000);
        ms_sleep(to_wait);
        iter++;
    }
    return 0;
}
