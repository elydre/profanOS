#ifndef TYPE_H
#define TYPE_H

#define uint32_t unsigned int
#define uint16_t unsigned short
#define uint8_t unsigned char

#define low_16(address) (uint16_t)((address) & 0xFFFF)
#define high_16(address) (uint16_t)(((address) >> 16) & 0xFFFF)

#define NULL 0

#endif

