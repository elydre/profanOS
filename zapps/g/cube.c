/*****************************************************************************\
|   === cube.c : 2024 ===                                                     |
|                                                                             |
|    Good performance 3D cube rotation                             .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

// @LINK: libpf, libpm

#include <profan/syscall.h>
#include <profan/math.h>
#include <profan/vgui.h>

#include <stdlib.h>
#include <unistd.h>

#define FOCAL_DISTANCE 150
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

point2_t project(point3_t point) {
    int x = point.x * FOCAL_DISTANCE / (point.z + FOCAL_DISTANCE + 256);
    int y = point.y * FOCAL_DISTANCE / (point.z + FOCAL_DISTANCE + 256);
    return (point2_t){x, y};
}

shape_t new_shape(void) {
    shape_t shape;
    shape.PointsCount = 8;
    shape.LinesCount = 12; // 18 for triangles
    shape.Points = malloc(sizeof(point3_t) * shape.PointsCount);
    shape.Lines = malloc(sizeof(line_t) * shape.LinesCount);
    shape.ScreenPoints = malloc(sizeof(point2_t) * shape.PointsCount);
    return shape;
}

shape_t cube(int size) {
    shape_t shape = new_shape();

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

    return shape;
}

void delete_shape(shape_t *shape) {
    free(shape->Points);
    free(shape->Lines);
    free(shape->ScreenPoints);
}

float cos_call(int angle) {
    // convert to radians
    float x = angle * M_PI / 180;
    // cos of x in radians
    return cosf(x);
}

float sin_call(int angle) {
    // convert to radians
    float x = angle * M_PI / 180;
    // sin of x in radians
    return sinf(x);
}

void rotate(shape_t *new_shape, shape_t *shape, int x, int y, int z) {
    /* new object is required because the
     * floating point math is not exact
     * and the shape will be deformed */
    int x1, y1, z1, i;
    for (i = 0; i < new_shape->PointsCount; i++) {
        x1 = shape->Points[i].x;
        y1 = shape->Points[i].y;
        z1 = shape->Points[i].z;
        new_shape->Points[i].x = x1 * cos_call(z) - y1 * sin_call(z);
        new_shape->Points[i].y = x1 * sin_call(z) + y1 * cos_call(z);
        x1 = new_shape->Points[i].x;
        y1 = new_shape->Points[i].y;
        new_shape->Points[i].x = x1 * cos_call(y) + z1 * sin_call(y);
        new_shape->Points[i].z = -x1 * sin_call(y) + z1 * cos_call(y);
        x1 = new_shape->Points[i].x;
        z1 = new_shape->Points[i].z;
        new_shape->Points[i].y = y1 * cos_call(x) - z1 * sin_call(x);
        new_shape->Points[i].z = y1 * sin_call(x) + z1 * cos_call(x);
    }
    for (i = 0; i < new_shape->LinesCount; i++) {
        new_shape->Lines[i] = shape->Lines[i];
    }

    for (i = 0; i < new_shape->PointsCount; i++) {
        new_shape->ScreenPoints[i] = project(new_shape->Points[i]);
    }
}

void draw(shape_t *shape, vgui_t *vgui, int erase) {
    for (int i = 0; i < shape->LinesCount; i++) {
        line_t line = shape->Lines[i];
        point2_t p1 = shape->ScreenPoints[line.i1];
        point2_t p2 = shape->ScreenPoints[line.i2];

        vgui_draw_line(vgui, p1.x+100, p1.y+100, p2.x+100, p2.y+100, erase ? 0 : line.color);
    }
}

int fps_limiter(int time) {
    int new_time = syscall_timer_get_ms();
    int fps = 1000 / (new_time - time + 1);
    if (fps > 30)
        usleep(10000);
    return new_time;
}

int main(void) {
    vgui_t vgui = vgui_setup(200, 200);

    shape_t shape = cube(120);
    int time = syscall_timer_get_ms();

    shape_t nshape = new_shape();
    for (int i = 0;; i = (i + 1) % 360) {
        rotate(&nshape, &shape, i, i * 2 % 360, i);
        draw(&nshape, &vgui, 0);
        vgui_render(&vgui, 0);
        draw(&nshape, &vgui, 1);
        time = fps_limiter(time);
    }

    delete_shape(&nshape);
    delete_shape(&shape);
    vgui_exit(&vgui);

    return 0;
}
