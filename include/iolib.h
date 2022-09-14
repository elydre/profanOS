#ifndef SKPRINT_H
#define SKPRINT_H

#include <driver/screen.h>

void mskprint(int nb_args, ...);
void fskprint(char format[], ...);
void rainbow_print(char message[]);

void input_paste(char out_buffer[], int size, char paste_buffer[], ScreenColor color);
void input(char out_buffer[], int size, ScreenColor color);

#endif
