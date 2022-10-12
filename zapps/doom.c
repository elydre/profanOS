#include "syscall.h"

#define mapsize 10
#define pi 3.14159
#define calc_speed 0.5

#define height_at_1m 3
#define minimap_size 4

#define player_speed 0.1
#define rot_speed 2
#define fov 30

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
    2, 7, 7, 7, 1, 7, 7, 7, 7, 7
};

double get_distance(double x, double y, double rad_angle, int * color);
void draw_rect_buffer(int x, int y, int width, int height, int color, int buffer_width, char * buffer);

double cos(double x);
double sin(double x);

double deg_to_rad(int x);

int main(int arg) {
    // 2.5D game doom like
    double x = 5, y = 5;
    double rot = 0; // in degrees

    int width = 320, height = 200;
    int half_height = height / 2;

    int center, top, bottom;
    double angle;

    int color;
    char * buffer = c_malloc(width * height);

    c_vga_320_mode();
    for (int tick = 4; c_kb_get_scancode() != 1; tick = (tick > 55) ? 4 : tick + 8) {
        for (int i = 0; i < width; i++) {
            angle = deg_to_rad(rot + (fov / 2) - (fov * i / width));

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
                if (i == (int) (x + cos(deg_to_rad(rot)) * 2) && j == (int) (y + sin(deg_to_rad(rot)) * 2))
                    draw_rect_buffer(i * minimap_size, j * minimap_size, minimap_size / 2, minimap_size / 2, 61, width, buffer);
            }
        }

        draw_rect_buffer(width - 5, 0, 5, 5, tick, width, buffer);

        for (int i = 0; i < width * height; i++) {
            c_vga_put_pixel(i % width, i / width, buffer[i]);
        }
        
        if (c_kb_get_scancode() == KB_D) {
            rot -= rot_speed;
        }
        if (c_kb_get_scancode() == KB_Q) {
            rot += rot_speed;
        }
        if (c_kb_get_scancode() == KB_Z) {
            x += cos(deg_to_rad(rot)) * player_speed;
            y += sin(deg_to_rad(rot)) * player_speed;
        }
        if (c_kb_get_scancode() == KB_S) {
            x -= cos(deg_to_rad(rot)) * player_speed;
            y -= sin(deg_to_rad(rot)) * player_speed;
        }

        if (rot > 360) rot -= 360;
        if (rot < 0) rot += 360;
        if (x < 1) x = 1;
        if (y < 1) y = 1;
        if (x > mapsize - 2) x = mapsize - 2;
        if (y > mapsize - 2) y = mapsize - 2;

        c_ms_sleep(20);
    }

    c_vga_text_mode();
    c_free(buffer);

    return arg;
}

double modd(double x, double y) {
    return x - (int) (x / y) * y;
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

double deg_to_rad(int x) {
    return x * pi / 180;
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