#include "syscall.h"

#define mapsize 10
#define pi 3.14159
#define calc_speed 0.5

#define height_at_1m 3
#define minimap_size 4

#define player_speed 0.1
#define rot_speed (pi / 25)
#define fov (pi / 4)

#define floor_color 0
#define ceiling_color 3

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
            angle = rot + (fov / 2) - (fov * i / width);

            center = (int) (half_height * height_at_1m / get_distance(x, y, angle, &color));
            top = (int) (half_height - center);
            bottom = (int) (half_height + center);

            for (int j = 0; j < height; j++) {
                if (j < top) buffer[i + j * width] = ceiling_color;
                else if (j > bottom) buffer[i + j * width] = floor_color;
                else buffer[i + j * width] = color;
            }
        }

        for (int i = 0; i < mapsize; i++) {
            for (int j = 0; j < mapsize; j++) {
                draw_rect_buffer(i * minimap_size, j * minimap_size, minimap_size, minimap_size, MAP[i + j * mapsize], width, buffer);
                if (i == (int) x && j == (int) y)
                    draw_rect_buffer(i * minimap_size, j * minimap_size, minimap_size, minimap_size, 36, width, buffer);
                if (i == (int) (x + cos(rot) * 2) && j == (int) (y + sin(rot) * 2))
                    draw_rect_buffer(i * minimap_size, j * minimap_size, minimap_size / 2, minimap_size / 2, 61, width, buffer);
            }
        }

        draw_rect_buffer(width - 5, 0, 5, 5, tick, width, buffer);

        for (int i = 0; i < width * height; i++) {
            c_vga_put_pixel(i % width, i / width, buffer[i]);
        }

        if (last_key != c_kb_get_scancode()) {
            last_key = c_kb_get_scancode();
            if (last_key < KB_released_value && !(val_in_buffer(last_key, 20, key_buffer))) {
                add_to_buffer(last_key, key_buffer);
            } else if (last_key >= KB_released_value) {
                remove_from_buffer(last_key - KB_released_value, key_buffer);
            }
        }
        
        if (val_in_buffer(KB_Q, 20, key_buffer)) {
            rot += rot_speed;
        }
        if (val_in_buffer(KB_D, 20, key_buffer)) {
            rot -= rot_speed;
        }
        if (val_in_buffer(KB_Z, 20, key_buffer)) {
            x += cos(rot) * player_speed;
            y += sin(rot) * player_speed;
        }
        if (val_in_buffer(KB_S, 20, key_buffer)) {
            x -= cos(rot) * player_speed;
            y -= sin(rot) * player_speed;
        }

        if (x < 1) x = 1;
        if (y < 1) y = 1;
        if (x > mapsize - 2) x = mapsize - 2;
        if (y > mapsize - 2) y = mapsize - 2;

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
    x = modd(x, pi);
    return 1 - ((x * x) / (2)) + ((x * x * x * x) / (24)) - ((x * x * x * x * x * x) / (720));
}

double sin(double x) {
    // sin of x in radians
    x = modd(x, pi);
    return x - ((x * x * x) / (6)) + ((x * x * x * x * x) / (120)) - ((x * x * x * x * x * x * x) / (5040));
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

        if (MAP[cell_y * mapsize + cell_x]) {
            * color = MAP[cell_y * mapsize + cell_x];
            return distance;
        }

        distance += 1 / max(abs(dx), abs(dy));

        px += dx * calc_speed;
        py += dy * calc_speed;
    }
    * color = 0;
    return distance;
}
