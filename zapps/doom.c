#include "syscall.h"

#define MAP_SIZE 10
#define PI 3.14159
#define CALC_SPEED 0.4
#define MATH_LOOP 10

#define BLOCK_HEIGHT 5
#define MINIMAP_SIZE 4

#define PLAYER_SPEED 0.1
#define ROT_SPEED (PI / 25)
#define FOV (PI / 4)

#define FLOOR_COLOR 0
#define CEILING_COLOR 3

int MAP[] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 6,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 6,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 6,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 6,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 6,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 6,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 6,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 6,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 6,
    2, 7, 7, 7, 9, 7, 7, 7, 7, 7
};

double get_distance(double x, double y, double rad_angle, int * color);
void draw_rect_buffer(int x, int y, int width, int height, int color, int buffer_width, char * buffer);

int val_in_buffer(int val, int buffer_width, int * buffer);
void remove_from_buffer(int val, int * buffer);
void add_to_buffer(int val, int * buffer);

double cos(double x);
double sin(double x);

int main(int arg) {
    // 2.5D game doom like
    double x = 5, y = 5;
    double rot = 0; // in radians

    int width = 320, height = 200;
    int half_height = height / 2;

    int center, top, bottom;
    double angle;

    int color, last_key = 0;
    char * buffer = c_malloc(width * height);
    int * key_buffer = c_calloc(20); // for init to 0

    c_vga_320_mode();
    for (int tick = 4; c_kb_get_scancode() != 1; tick = (tick > 55) ? 4 : tick + 8) {
        for (int i = 0; i < width; i++) {
            angle = rot + (FOV / 2) - (FOV * i / width);

            center = (int) (half_height * BLOCK_HEIGHT / get_distance(x, y, angle, &color));
            top = (int) (half_height - center);
            bottom = (int) (half_height + center);

            for (int j = 0; j < height; j++) {
                if (j < top) buffer[i + j * width] = CEILING_COLOR;
                else if (j > bottom) buffer[i + j * width] = FLOOR_COLOR;
                else buffer[i + j * width] = color;
            }
        }

        for (int i = 0; i < MAP_SIZE; i++) {
            for (int j = 0; j < MAP_SIZE; j++) {
                draw_rect_buffer(i * MINIMAP_SIZE, j * MINIMAP_SIZE, MINIMAP_SIZE, MINIMAP_SIZE, MAP[i + j * MAP_SIZE], width, buffer);
                if (i == (int) x && j == (int) y)
                    draw_rect_buffer(i * MINIMAP_SIZE, j * MINIMAP_SIZE, MINIMAP_SIZE, MINIMAP_SIZE, 36, width, buffer);
                if (i == (int) (x + cos(rot) * 2) && j == (int) (y + sin(rot) * 2))
                    draw_rect_buffer(i * MINIMAP_SIZE, j * MINIMAP_SIZE, MINIMAP_SIZE / 2, MINIMAP_SIZE / 2, 61, width, buffer);
            }
        }

        draw_rect_buffer(width - 5, 0, 5, 5, tick, width, buffer);

        for (int i = 0; i < width * height; i++) {
            c_vga_put_pixel(i % width, i / width, buffer[i]);
        }

        if (last_key != c_kb_get_scancode()) {
            last_key = c_kb_get_scancode();
            if (last_key == KB_ESC) {
                for (int i = 0; i < 20; i++) key_buffer[i] = 0;
            }
            if (last_key < KB_released_value && !(val_in_buffer(last_key, 20, key_buffer))) {
                add_to_buffer(last_key, key_buffer);
            } else if (last_key >= KB_released_value) {
                remove_from_buffer(last_key - KB_released_value, key_buffer);
            }
        }
        
        if (val_in_buffer(KB_Q, 20, key_buffer)) {
            rot += ROT_SPEED;
        }
        if (val_in_buffer(KB_D, 20, key_buffer)) {
            rot -= ROT_SPEED;
        }
        if (val_in_buffer(KB_Z, 20, key_buffer)) {
            x += cos(rot) * PLAYER_SPEED;
            y += sin(rot) * PLAYER_SPEED;
        }
        if (val_in_buffer(KB_S, 20, key_buffer)) {
            x -= cos(rot) * PLAYER_SPEED;
            y -= sin(rot) * PLAYER_SPEED;
        }

        if (x < 1) x = 1;
        if (y < 1) y = 1;
        if (x > MAP_SIZE - 2) x = MAP_SIZE - 2;
        if (y > MAP_SIZE - 2) y = MAP_SIZE - 2;

        c_ms_sleep(10);
    }

    c_vga_text_mode();

    c_free(buffer);
    c_free(key_buffer);

    return arg;
}

double modd(double x, double y) {
    // mod function for double and negative numbers
    double res = x - (int) (x / y) * y;
    if (res < 0) res += y;
    return res;
}

double cos(double x) {
    // cos of x in radians
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
    // sin of x in radians
    double res = x;
    double pow = x;
    double fact = 1;
    for (int i = 0; i < MATH_LOOP; i++) {
        pow *= -1 * x * x;
        fact *= (2 * i + 2) * (2 * i + 3);
        res += pow / fact;
    }
    return res;    
}

double max(double a, double b) {
    return (a > b) ? a : b;
}

double abs(double a) {
    return (a < 0) ? -a : a;
}

int val_in_buffer(int val, int buffer_width, int * buffer) {
    for (int i = 0; i < buffer_width; i++) {
        if (buffer[i] == val) return 1;
    }
    return 0;
}

void add_to_buffer(int val, int * buffer) {
    for (int i = 0; 1; i++) {
        if (buffer[i] == 0) {
            buffer[i] = val;
            return;
        }
    }
}

void remove_from_buffer(int val, int * buffer) {
    for (int i = 0; i < 20; i++) {
        if (buffer[i] == val) {
            buffer[i] = 0;
            return;
        }
    }
}

void draw_rect_buffer(int x, int y, int width, int height, int color, int buffer_width, char * buffer) {
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            buffer[(x + i) + (y + j) * buffer_width] = color;
        }
    }
}

double get_distance(double x, double y, double rad_angle, int * color) {
    double dx = cos(rad_angle);
    double dy = sin(rad_angle);

    double px = x;
    double py = y;

    double distance = 0;

    int cell_x, cell_y;

    for (int i = 0; i < 100; i++) {
        cell_x = (int) px;
        cell_y = (int) py;

        if (MAP[cell_y * MAP_SIZE + cell_x]) {
            * color = MAP[cell_y * MAP_SIZE + cell_x];
            return distance;
        }

        distance += 1 / max(abs(dx), abs(dy));

        px += dx * CALC_SPEED;
        py += dy * CALC_SPEED;
    }
    * color = 0;
    return distance;
}
