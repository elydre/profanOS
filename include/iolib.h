#ifndef SKPRINT_H
#define SKPRINT_H

#include <driver/screen.h>

void mskprint(int nb_args, ...);
void fskprint(char format[], ...);
void input(char out_buffer[], int size, ScreenColor color);
void rainbow_print(char message[]);

#endif
