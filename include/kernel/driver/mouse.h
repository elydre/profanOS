#ifndef MOUSE_H
#define MOUSE_H

#define MOUSE_FLAG_L_BUTTON 0x1
#define MOUSE_FLAG_R_BUTTON 0x2
#define MOUSE_FLAG_M_BUTTON 0x4

#include <cpu/isr.h>
#include <type.h>

typedef struct {
	uint8_t flags, x_mov, y_mov, z_mov;
} mouse_packet_t;

int mouse_init();

int mouse_call(int thing, int val);

uint8_t mouse_is_available();

#endif
