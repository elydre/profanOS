#include "syscall.h"

#define mapsize 10
#define pi 3.14159
#define player_speed 0.3
#define fov (pi / 4)

#define wall_color 4
#define floor_color 0
#define ceiling_color 3

int map[] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 0, 0, 0, 1, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 0, 0, 1, 1, 1, 0, 0, 0, 0,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

float sqrt(float x);
float get_distance(float x1, float y1, float x2, float y2);
void calc_slice_height(int * slice_height, int width, int height, float player_x, float player_y, float rot);

float cos(float x);
float sin(float x);

int main(int arg) {
    // 2.5D game doom like
    float x = 5, y = 5, rot = 0;
    int test = 0;

    int width = 320, height = 200;
    int half_height = height / 2;

    int slice_height[width];
    c_vga_320_mode();
    while (c_kb_get_scancode() != 1) {
        calc_slice_height(slice_height, width, height, x, y, rot);
        for (int i = 0; i < width; i++) {
            int center = slice_height[i];
            int top = half_height - center;
            int bottom = half_height + center;

            c_vga_draw_rect(i, 0, 1, top, ceiling_color);
            c_vga_draw_rect(i, top, 1, center * 2, wall_color);
            c_vga_draw_rect(i, bottom, 1, height - bottom, floor_color);
        }
        
        if (c_kb_get_scancode() == KB_D) {
            rot += 0.1;
        }
        if (c_kb_get_scancode() == KB_Q) {
            rot -= 0.1;
        }
        if (c_kb_get_scancode() == KB_Z) {
            x += cos(rot) * player_speed;
            y += sin(rot) * player_speed;
        }
        if (c_kb_get_scancode() == KB_S) {
            x -= cos(rot) * player_speed;
            y -= sin(rot) * player_speed;
        }
        test = !test;
        c_vga_draw_rect(0, 0, 10, 10, test + 5);
    }

    c_vga_text_mode();

    return arg;
}

float sqrt(float x) {
    // quake 3
    float xhalf = 0.5f * x;
    int i = *(int*)&x;
    i = 0x5f3759df - (i >> 1);
    x = *(float*)&i;
    x = x * (1.5f - xhalf * x * x);
    return 1 / x;
}

float get_distance(float x1, float y1, float x2, float y2) {
    float dx = x1 - x2;
    float dy = y1 - y2;
    return sqrt(dx * dx + dy * dy);
}

int factorial(int n) {
    if (n == 0) return 1;
    return n * factorial(n - 1);
}

float cos(float x) {
    // calculate the cos of x
    int i;
    float y = 0;
    for (i = 0; i < 10; i++) {
        y += c_pow(-1, i) * c_pow(x, 2 * i) / factorial(2 * i);
    }
    return y;
}

float sin(float x) {
    // calculate the sin of x
    int i;
    float y = 0;
    for (i = 0; i < 10; i++) {
        y += c_pow(-1, i) * c_pow(x, 2 * i + 1) / factorial(2 * i + 1);
    }
    return y;
}

void calc_slice_height(int * slice_height, int width, int height, float player_x, float player_y, float rot) {
    // edit slice_height array with the height of each slice of the screen
    for (int i = 0; i < width; i++) {
        float ray_angle = rot - fov / 2 + fov * i / width;
        float step_size = 0.1;
        float distance_to_wall = 0;

        int hit_wall = 0;
        while (!hit_wall && distance_to_wall < 10) {
            distance_to_wall += step_size;
            int test_x = (int)(player_x + cos(ray_angle) * distance_to_wall);
            int test_y = (int)(player_y + sin(ray_angle) * distance_to_wall);

            if (test_x < 0 || test_x >= mapsize ||
                test_y < 0 || test_y >= mapsize) {
                hit_wall = 1;
                distance_to_wall = 10;
            } else {
                if (map[test_x + test_y * mapsize] == 1) {
                    hit_wall = 1;
                }
            }
        }
        slice_height[i] = (int)(height / distance_to_wall);
    }
}
