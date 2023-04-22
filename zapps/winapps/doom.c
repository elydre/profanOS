#include <syscall.h>
#include <stdlib.h>

#include <i_libdaube.h>
#include <i_winadds.h>

#define MAP_SIZE 10
#define PI 3.141592
#define MATH_LOOP 7
#define ONK 1000.0

#define BLOCK_HEIGHT 2
#define MINIMAP_SIZE 4

#define PLAYER_SPEED 5
#define ROT_SPEED 3
#define FOV (PI / 4)

#define FLOOR_COLOR 0x000044
#define CEILING_COLOR 0x66FFFF

int MAP[] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 6,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 6,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 6,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 6,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 6,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 6,
    2, 0, 0, 0, 0, 0, 5, 0, 0, 6,
    2, 0, 0, 0, 0, 0, 5, 0, 0, 6,
    2, 0, 0, 0, 0, 0, 5, 0, 0, 6,
    2, 7, 7, 7, 9, 7, 7, 7, 7, 7
};

int is_running;

double get_distance(double x, double y, double rad_angle, int *color);

int val_in_buffer(int val, int buffer_width, int *buffer);
void remove_from_buffer(int val, int *buffer);
void add_to_buffer(int val, int *buffer);
uint32_t convert_color(int color);

double cos(double x);
double sin(double x);

void exit_callback(clickevent_t *event) {
    is_running = 0;
}

int main(int argc, char **argv) {
    // wake up the parent process
    c_process_wakeup(c_process_get_ppid(c_process_get_pid()));

    double x = 5, y = 5;
    double rot = 0; // in radians

    int width = 320, height = 200;
    int half_height = height / 2;

    // get the main desktop
    desktop_t *main_desktop = desktop_get_main();

    // create a window and add an exit button
    window_t *window = window_create(main_desktop, "doom like", 100, 100, width, height, 0, 0, 0);
    wadds_create_exitbt(window, exit_callback);
    desktop_refresh(main_desktop);

    int center, top, bottom;
    char convert[10];

    int color, last_key, key = 0;
    int key_buffer[20];
    for (int i = 0; i < 20; i++) key_buffer[i] = 0;
    int tick_count[4];
    tick_count[0] = c_timer_get_ms();
    tick_count[3] = 0;

    c_kb_reset_history();
    for (int i = 0; i < 100; i++) c_kb_get_scfh();

    is_running = 1;
    while (is_running) {
        tick_count[1] = c_timer_get_ms() - tick_count[0];
        tick_count[0] = c_timer_get_ms();

        for (int i = 0; i < width; i++) {
            center = (int) (half_height * BLOCK_HEIGHT / get_distance(x, y, rot + (FOV / 2) - (FOV * i / width), &color));
            top = (int) (half_height - center);
            bottom = (int) (half_height + center);

            for (int j = 0; j < height; j++) {
                if (j < top) window_set_pixel(window, i, j, CEILING_COLOR);
                else if (j > bottom) window_set_pixel(window, i, j, FLOOR_COLOR);
                else window_set_pixel(window, i, j, convert_color(color));
            }
        }

        for (int i = 0; i < MAP_SIZE; i++) {
            for (int j = 0; j < MAP_SIZE; j++) {
                wadds_rect(window, width - MINIMAP_SIZE * MAP_SIZE + i * MINIMAP_SIZE, j * MINIMAP_SIZE, MINIMAP_SIZE, MINIMAP_SIZE, convert_color(MAP[i + j * MAP_SIZE]));
                if (i == (int) x && j == (int) y)
                    wadds_rect(window, width - MINIMAP_SIZE * MAP_SIZE + i * MINIMAP_SIZE, j * MINIMAP_SIZE, MINIMAP_SIZE, MINIMAP_SIZE, 0xFFFFFF);
                if (i == (int)(x + cos(rot) * 2) && j == (int)(y + sin(rot) * 2))
                    wadds_rect(window, width - MINIMAP_SIZE * MAP_SIZE + i * MINIMAP_SIZE, j * MINIMAP_SIZE, MINIMAP_SIZE / 2, MINIMAP_SIZE / 2, 0x00FF00);
            }
        }

        wadds_rect(window, 0, 0, tick_count[1] * 2, 7, 0x880000);
        wadds_rect(window, 0, 0, (tick_count[1] - tick_count[3]) * 2, 7, 0xCC0000);

        itoa(1000 / (tick_count[1] + 1), convert, 10);
        wadds_puts(window, convert, 0, 8, 0x000000, 0xFF000000);
        
        tick_count[2] = c_timer_get_ms();
        window_refresh(window);
        tick_count[3] = c_timer_get_ms() - tick_count[2];

        key = c_kb_get_scfh();
        if (last_key != key && key != 0) {
            last_key = key;
            if (last_key < KB_released_value && !(val_in_buffer(last_key, 20, key_buffer))) {
                add_to_buffer(last_key, key_buffer);
            } else if (last_key >= KB_released_value) {
                remove_from_buffer(last_key - KB_released_value, key_buffer);
            }
        }

        if (val_in_buffer(KB_Q, 20, key_buffer)) {
            rot += ROT_SPEED * tick_count[1] / ONK;
        }

        if (val_in_buffer(KB_D, 20, key_buffer)) {
            rot -= ROT_SPEED * tick_count[1] / ONK;
        }

        if (val_in_buffer(KB_Z, 20, key_buffer)) {
            x += cos(rot) * PLAYER_SPEED * tick_count[1] / ONK;
            y += sin(rot) * PLAYER_SPEED * tick_count[1] / ONK;
        }

        if (val_in_buffer(KB_S, 20, key_buffer)) {
            x -= cos(rot) * PLAYER_SPEED * tick_count[1] / ONK;
            y -= sin(rot) * PLAYER_SPEED * tick_count[1] / ONK;
        }

        if (val_in_buffer(KB_A, 20, key_buffer)) {
            x += cos(rot + PI / 2) * PLAYER_SPEED * tick_count[1] / ONK;
            y += sin(rot + PI / 2) * PLAYER_SPEED * tick_count[1] / ONK;
        }

        if (val_in_buffer(KB_E, 20, key_buffer)) {
            x += cos(rot - PI / 2) * PLAYER_SPEED * tick_count[1] / ONK;
            y += sin(rot - PI / 2) * PLAYER_SPEED * tick_count[1] / ONK;
        }

        if (x < 1) x = 1;
        if (y < 1) y = 1;
        if (x > MAP_SIZE - 2) x = MAP_SIZE - 2;
        if (y > MAP_SIZE - 2) y = MAP_SIZE - 2;

        if (rot > PI) rot -= 2 * PI;
        if (rot < -PI) rot += 2 * PI;
    }
    
    // destroy window and wait for it to be deleted
    window_delete(window);
    window_wait_delete(main_desktop, window);

    return 0;
}

double cos(double x) {
    // cos of x in radians
    while (x > PI) x -= 2 * PI;
    while (x < -PI) x += 2 * PI;
    double res = 1;
    double pow = 1;
    double fact = 1;
    for (int i = 0; i < MATH_LOOP; i++) {
        pow *= -1 * x * x;
        fact *= (2 * i + 1) * (2 * i + 2);
        res += pow / fact;
    }
    return res;
}

double sin(double x) {
    return cos(x - PI / 2); 
}

int val_in_buffer(int val, int buffer_width, int *buffer) {
    for (int i = 0; i < buffer_width; i++) {
        if (buffer[i] == val) return 1;
    }
    return 0;
}

void add_to_buffer(int val, int *buffer) {
    for (int i = 0; 1; i++) {
        if (buffer[i] == 0) {
            buffer[i] = val;
            return;
        }
    }
}

uint32_t convert_color(int color) {
    switch (color) {
        case 0: return 0x000000;
        case 1: return 0x0000AA;
        case 2: return 0x00AA00;
        case 3: return 0x00AAAA;
        case 4: return 0xAA0000;
        case 5: return 0xAA00AA;
        case 6: return 0xAA5500;
        case 7: return 0xAAAAAA;
        case 8: return 0x555555;
        case 9: return 0x5555FF;
        case 10: return 0x55FF55;
        case 11: return 0x55FFFF;
        case 12: return 0xFF5555;
        case 13: return 0xFF55FF;
        case 14: return 0xFFFF55;
        case 15: return 0xFFFFFF;
    }
    return 0;
}

void remove_from_buffer(int val, int *buffer) {
    for (int i = 0; i < 20; i++) {
        if (buffer[i] == val) {
            buffer[i] = 0;
            return;
        }
    }
}

double get_distance(double x, double y, double rad_angle, int *color) {
    double dx = cos(rad_angle);
    double dy = sin(rad_angle);

    double distance = 0;
    while (1) {
        distance += 0.1;
        int map_x = (int) (x + dx * distance);
        int map_y = (int) (y + dy * distance);
        if (map_x < 0 || map_x >= MAP_SIZE || map_y < 0 || map_y >= MAP_SIZE) {
            *color = 0;
            return distance;
        }
        if (MAP[map_x + map_y * MAP_SIZE] > 0) {
            *color = MAP[map_x + map_y * MAP_SIZE];
            return distance;
        }
    }
}
