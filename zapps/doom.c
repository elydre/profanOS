#include "syscall.h"

#define mapsize 10
#define pi 3.14159
#define calc_speed 0.5

#define height_at_1m 3
#define player_speed 0.3
#define rot_speed 5
#define fov (pi / 3)

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
    2, 7, 7, 7, 7, 7, 7, 7, 7, 7
};

double get_distance(double x, double y, double rad_angle, int * color);

double cos(double x);
double sin(double x);

double deg_to_rad(int x);

int main(int arg) {
    // 2.5D game doom like
    double x = 5, y = 5;
    double rot = 0; // in degrees
    char temp[5];


    int width = 320, height = 200;
    int half_height = height / 2;

    int center, top, bottom;
    double part_height;

    int color;

    c_vga_320_mode();
    while (c_kb_get_scancode() != 1) {
        for (int i = 0; i < width; i++) {
            double angle = deg_to_rad(rot + (fov / 2) - (fov * i / width));

            part_height = height_at_1m / get_distance(x, y, angle, &color);
            center = (int) (half_height * part_height);
            top = (int) (half_height - center);
            bottom = (int) (half_height + center);

            for (int j = 0; j < top; j++) {
                c_vga_put_pixel(i, j, ceiling_color);
            }
            for (int j = top; j < bottom; j++) {
                c_vga_put_pixel(i, j, color);
            }
            for (int j = bottom; j < height; j++) {
                c_vga_put_pixel(i, j, floor_color);
            }
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

        // draw minimap
        for (int i = 0; i < mapsize * mapsize; i++) {
            c_vga_draw_rect(width - mapsize * 2 + i % mapsize * 2, i / mapsize * 2, 2, 2, MAP[i]);
        }

        // draw player
        c_vga_draw_rect(width - mapsize * 2 + (int) x * 2, (int) y * 2, 2, 2, 63);

        // draw player direction
        c_vga_draw_rect(width - mapsize * 2 + (int) (x + cos(deg_to_rad(rot)) * 2) * 2, (int) (y + sin(deg_to_rad(rot)) * 2) * 2, 2, 2, 61);

        c_int_to_ascii((int) rot, temp);
        c_vga_print(width - 30, height - 10, temp, 0, 63);

        c_ms_sleep(20);
    }

    c_vga_text_mode();

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
