#ifndef SKPRINT_H
#define SKPRINT_H

#include <driver/screen.h>

void mskprint(int nb_args, ...);
void fskprint(char format[], ...);
void rainbow_print(char message[]);

void input_wh(char out_buffer[], int size, ScreenColor color, char ** history, int history_size);
void input(char out_buffer[], int size, ScreenColor color);

#endif
