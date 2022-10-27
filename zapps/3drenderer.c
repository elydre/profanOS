#include <syscall.h>

typedef struct Point3_t {
    int x;
    int y;
    int z;
} Point3_t;

typedef struct Line_t {
    int i1;
    int i2;
} Line_t;

typedef struct Shape_t {
    Point3_t* Points;
    Line_t* Lines;
    int PointsCount;
    int LinesCount;
} Shape_t;

Shape_t cube(int size, Point3_t origine);
float cos(int angle);
float sin(int angle);

int main(int argc, char** argv) {
    c_vga_320_mode();

    int size_buffer = 1;
    Shape_t shape = cube(10, (Point3_t){0, 0, 0});

    c_vgui_setup(0);

    c_vgui_render();

    while (1);

    c_vga_text_mode();

    return 0;
}

Shape_t cube(int size, Point3_t origine) {
    Shape_t shape;
    shape.PointsCount = 8;
    shape.LinesCount = 12;
    shape.Points = c_malloc(sizeof(Point3_t) * shape.PointsCount);
    shape.Lines = c_malloc(sizeof(Line_t) * shape.LinesCount);

    shape.Points[0] = (Point3_t){origine.x, origine.y, origine.z};
    shape.Points[1] = (Point3_t){origine.x + size, origine.y, origine.z};
    shape.Points[2] = (Point3_t){origine.x + size, origine.y + size, origine.z};
    shape.Points[3] = (Point3_t){origine.x, origine.y + size, origine.z};
    shape.Points[4] = (Point3_t){origine.x, origine.y, origine.z + size};
    shape.Points[5] = (Point3_t){origine.x + size, origine.y, origine.z + size};
    shape.Points[6] = (Point3_t){origine.x + size, origine.y + size, origine.z + size};
    shape.Points[7] = (Point3_t){origine.x, origine.y + size, origine.z + size};

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

