#ifndef SKPRINT_H
#define SKPRINT_H

#include <gui/gnrtx.h>

void mskprint(int nb_args, ...);
void fskprint(char format[], ...);
void rainbow_print(char message[]);

void input_wh(char out_buffer[], int size, char color, char ** history, int history_size);
void input(char out_buffer[], int size, char color);

#endif
