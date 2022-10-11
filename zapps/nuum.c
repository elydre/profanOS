#include "syscall.h"

#define mapsize 10
#define pi 3.14159
#define player_speed 0.2
#define fov (pi / 4)

#define wall_color 4
#define floor_color 3
#define ceiling_color 0

int map[] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 0, 0, 0, 1, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 1, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

float sqrt(float x);
float get_distance(float x1, float y1, float x2, float y2);
void calc_slice_height(int * slice_height, int height, float player_x, float player_y, float rot);

int main(int arg) {
    // 2.5D game doom like
    float x = 1.5, y = 1.5, rot = 0;
    float angle;

    int width = 320, height = 200;
    int half_height = height / 2;

    int slice_height[width];

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

void calc_slice_height(int * slice_height, int width, float player_x, float player_y, float rot) {
    int i;
    float angle;
    float distance;
    float x, y;
    int map_x, map_y;
    int wall_height;

    for (i = 0; i < width; i++) {
        angle = rot - fov / 2 + fov * i / width;
        x = player_x;
        y = player_y;
        distance = 0;
        while (1) {
            x += player_speed * cos(angle);
            y += player_speed * sin(angle);
            distance += player_speed;
            map_x = (int)x;
            map_y = (int)y;
            if (map[map_x + map_y * mapsize] == 1) {
                break;
            }
        }
        wall_height = (int)(height / distance);
        slice_height[i] = wall_height;
    }
}