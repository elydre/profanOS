#include <syscall.h>

typedef struct Point3_t {
    int x;
    int y;
    int z;
} Point3_t;

typedef struct Point2_t {
    int x;
    int y;
} Point2_t;

typedef struct Line_t {
    int i1;
    int i2;
} Line_t;

typedef struct Shape_t {
    Point3_t* Points;
    Line_t* Lines;
    Point2_t* ScreenPoints;
    int PointsCount;
    int LinesCount;
} Shape_t;

Point2_t project(Point3_t point, int focal_distance);
Shape_t cube(int size);
float cos(int angle);
float sin(int angle);

int main(int argc, char** argv) {
    c_vga_320_mode();
    c_vgui_setup(0);

    Shape_t shape = cube(100);
    int focal_distance = 50;

    shape.ScreenPoints = c_malloc(sizeof(Point2_t) * shape.PointsCount);
    for (int i=0; i<shape.PointsCount; i++) {
        Point2_t p = project(shape.Points[i], focal_distance);
        shape.ScreenPoints[i] = p;
    }
    for (int i=0; i<shape.LinesCount; i++) {
        Line_t line = shape.Lines[i];
        Point2_t p1 = shape.ScreenPoints[line.i1];
        Point2_t p2 = shape.ScreenPoints[line.i2];
        c_vgui_draw_line(p1.x+100, p1.y+100, p2.x+100, p2.y+100, 63);
    }

    c_vgui_render();

    while (1);
    c_vga_text_mode();
    return 0;
}

Point2_t project(Point3_t point, int focal_distance) {
    int x = point.x * focal_distance / (point.z + focal_distance + 256);
    int y = point.y * focal_distance / (point.z + focal_distance + 256   );
    return (Point2_t){x, y};
}

Shape_t cube(int size) {
    Shape_t shape;
    shape.PointsCount = 8;
    shape.LinesCount = 12;
    shape.Points = c_malloc(sizeof(Point3_t) * shape.PointsCount);
    shape.Lines = c_malloc(sizeof(Line_t) * shape.LinesCount);

    shape.Points[0] = (Point3_t){ size,  size,  size};
    shape.Points[1] = (Point3_t){ size, -size,  size};
    shape.Points[2] = (Point3_t){-size, -size,  size};
    shape.Points[3] = (Point3_t){-size,  size,  size};
    shape.Points[4] = (Point3_t){ size,  size, -size};
    shape.Points[5] = (Point3_t){ size, -size, -size};
    shape.Points[6] = (Point3_t){-size, -size, -size};
    shape.Points[7] = (Point3_t){-size,  size, -size};

    shape.Lines[0] = (Line_t){0, 1};
    shape.Lines[1] = (Line_t){1, 2};
    shape.Lines[2] = (Line_t){2, 3};
    shape.Lines[3] = (Line_t){3, 0};
    shape.Lines[4] = (Line_t){4, 5};
    shape.Lines[5] = (Line_t){5, 6};
    shape.Lines[6] = (Line_t){6, 7};
    shape.Lines[7] = (Line_t){7, 4};
    shape.Lines[8] = (Line_t){0, 4};
    shape.Lines[9] = (Line_t){1, 5};
    shape.Lines[10] = (Line_t){2, 6};
    shape.Lines[11] = (Line_t){3, 7};

    return shape;
}

