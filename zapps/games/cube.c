#include <syscall.h>
#include <i_time.h>
#include <i_vgui.h>
#include <stdlib.h>


#define PI 3.141592
#define MATH_LOOP 100
#define FOCAL_DISTANCE 100
#define CUBE_COLOR 0xFFdd99
#define TRIN_COLOR 0xFFFF00

typedef struct point3_t {
    int x;
    int y;
    int z;
} point3_t;

typedef struct point2_t {
    int x;
    int y;
} point2_t;

typedef struct line_t {
    int i1;
    int i2;
    uint32_t color;
} line_t;

typedef struct shape_t {
    point3_t* Points;
    line_t* Lines;
    point2_t* ScreenPoints;
    int PointsCount;
    int LinesCount;
} shape_t;

shape_t cube(int size);
void delete_shape(shape_t *shape);

shape_t rotate(shape_t *shape, int x, int y, int z);
void draw(shape_t *shape, vgui_t *vgui);
int show_fps(vgui_t *vgui, int time);

int main(int argc, char** argv) {
    vgui_t vgui = vgui_setup(200, 200);

    shape_t shape = cube(120);
    int time;

    for (int i = 0; c_kb_get_scancode() != 1; i = (i + 1) % 360) {
        shape_t new_shape = rotate(&shape, i, i, i);
        draw(&new_shape, &vgui);
        delete_shape(&new_shape);
        time = show_fps(&vgui, time);

        vgui_render(&vgui, 0);
    }

    delete_shape(&shape);
    vgui_exit(&vgui);

    return 0;
}

point2_t project(point3_t point) {
    int x = point.x * FOCAL_DISTANCE / (point.z + FOCAL_DISTANCE + 256);
    int y = point.y * FOCAL_DISTANCE / (point.z + FOCAL_DISTANCE + 256);
    return (point2_t){x, y};
}

shape_t cube(int size) {
    shape_t shape;
    shape.PointsCount = 8;
    shape.LinesCount = 12; // 18 for triangles
    shape.Points = malloc(sizeof(point3_t) * shape.PointsCount);
    shape.Lines = malloc(sizeof(line_t) * shape.LinesCount);

    shape.Points[0] = (point3_t){ size,  size,  size};
    shape.Points[1] = (point3_t){ size, -size,  size};
    shape.Points[2] = (point3_t){-size, -size,  size};
    shape.Points[3] = (point3_t){-size,  size,  size};
    shape.Points[4] = (point3_t){ size,  size, -size};
    shape.Points[5] = (point3_t){ size, -size, -size};
    shape.Points[6] = (point3_t){-size, -size, -size};
    shape.Points[7] = (point3_t){-size,  size, -size};

    shape.Lines[0] = (line_t){0, 1, CUBE_COLOR};
    shape.Lines[1] = (line_t){1, 2, CUBE_COLOR};
    shape.Lines[2] = (line_t){2, 3, CUBE_COLOR};
    shape.Lines[3] = (line_t){3, 0, CUBE_COLOR};
    shape.Lines[4] = (line_t){4, 5, CUBE_COLOR};
    shape.Lines[5] = (line_t){5, 6, CUBE_COLOR};
    shape.Lines[6] = (line_t){6, 7, CUBE_COLOR};
    shape.Lines[7] = (line_t){7, 4, CUBE_COLOR};
    shape.Lines[8] = (line_t){0, 4, CUBE_COLOR};
    shape.Lines[9] = (line_t){1, 5, CUBE_COLOR};
    shape.Lines[10] = (line_t){2, 6, CUBE_COLOR};
    shape.Lines[11] = (line_t){3, 7, CUBE_COLOR};

    /* make squares into triangles
    shape.Lines[12] = (line_t){0, 2, TRIN_COLOR};
    shape.Lines[13] = (line_t){4, 6, TRIN_COLOR};
    shape.Lines[14] = (line_t){0, 5, TRIN_COLOR};
    shape.Lines[15] = (line_t){1, 6, TRIN_COLOR};
    shape.Lines[16] = (line_t){2, 7, TRIN_COLOR};
    shape.Lines[17] = (line_t){3, 4, TRIN_COLOR};
    */

    shape.ScreenPoints = malloc(sizeof(point2_t) * shape.PointsCount);
    return shape;
}

void delete_shape(shape_t *shape) {
    free(shape->Points);
    free(shape->Lines);
    free(shape->ScreenPoints);
}

double cos(int angle) {
    // convert to radians
    double x = angle * PI / 180;
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

double sin(int angle) {
    // convert to radians
    double x = angle * PI / 180;
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

shape_t rotate(shape_t *shape, int x, int y, int z) {
    /* new object is required because the
     * floating point math is not exact
     * and the shape will be deformed */
    shape_t new_shape;
    new_shape.PointsCount = shape->PointsCount;
    new_shape.LinesCount = shape->LinesCount;
    new_shape.Points = malloc(sizeof(point3_t) * new_shape.PointsCount);
    new_shape.Lines = malloc(sizeof(line_t) * new_shape.LinesCount);
    new_shape.ScreenPoints = malloc(sizeof(point2_t) * new_shape.PointsCount);
    int x1, y1, z1;
    for (int i = 0; i < new_shape.PointsCount; i++) {
        x1 = shape->Points[i].x;
        y1 = shape->Points[i].y;
        z1 = shape->Points[i].z;
        new_shape.Points[i].x = x1 * cos(z) - y1 * sin(z);
        new_shape.Points[i].y = x1 * sin(z) + y1 * cos(z);
        x1 = new_shape.Points[i].x;
        y1 = new_shape.Points[i].y;
        new_shape.Points[i].x = x1 * cos(y) + z1 * sin(y);
        new_shape.Points[i].z = -x1 * sin(y) + z1 * cos(y);
        x1 = new_shape.Points[i].x;
        z1 = new_shape.Points[i].z;
        new_shape.Points[i].y = y1 * cos(x) - z1 * sin(x);
        new_shape.Points[i].z = y1 * sin(x) + z1 * cos(x);
    }
    for (int i = 0; i < new_shape.LinesCount; i++) {
        new_shape.Lines[i] = shape->Lines[i];
    }
    return new_shape;
}

void draw(shape_t *shape, vgui_t *vgui) {
    vgui_clear(vgui, 0);
    for (int i=0; i<shape->PointsCount; i++) {
        point2_t p = project(shape->Points[i]);
        shape->ScreenPoints[i] = p;
    }
    for (int i=0; i<shape->LinesCount; i++) {
        line_t line = shape->Lines[i];
        point2_t p1 = shape->ScreenPoints[line.i1];
        point2_t p2 = shape->ScreenPoints[line.i2];

        vgui_draw_line(vgui, p1.x+100, p1.y+100, p2.x+100, p2.y+100, line.color);
    }
}

int show_fps(vgui_t *vgui, int time) {
    int new_time = c_timer_get_ms();
    int fps = 1000 / (new_time - time + 1);
    char fps_str[10];
    itoa(fps, fps_str, 10);
    vgui_print(vgui, 0, 0, fps_str, 0xFFFFFF);
    if (fps > 40)
        ms_sleep(10);
    return new_time;
}
