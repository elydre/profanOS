// @LINK SHARED: libm

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <math.h>
#include <string.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;

// Fonction pour envoyer une valeur à un port donné
void outb(uint16_t port, uint8_t value) {
	asm volatile ("outb %0, %1" :: "a"(value), "Nd"(port));
}

// Fonction pour lire une valeur d'un port donné
u8 inb(u16 port) {
    u8 value;
    asm volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

// Arrêter la génération de son
void stop() {
    u8 tmp = inb(0x61);
    outb(0x61, tmp & 0xFC);
}

// Définir la fréquence du son via le PIT (Programmable Interval Timer)
void freq(u32 frequency) {
    if (frequency == 0) {
        stop();
        return;
    }

    // Calcul de la division pour le PIT
    u32 divisor = 1193180 / frequency;

    // Configurer le PIT pour le mode 3 (génération de fréquence)
    outb(0x43, 0xB6);  // Commande pour le PIT
    outb(0x42, divisor & 0xFF);        // Partie basse de la division
    outb(0x42, (divisor >> 8) & 0xFF); // Partie haute de la division

    // Activer le haut-parleur
    u8 tmp = inb(0x61);
    if ((tmp & 3) != 3) {
        outb(0x61, tmp | 3);
    }
}

typedef struct {
	u32 frequency;
	u32 duration_ms;
} note_t;

u32 get_real_duration_ms(u32 duration, u32 bpm) {
	return duration * 60 * 1000 / bpm;
}

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <song file>\n", argv[0]);
		return 1;
	}

	FILE *file = fopen(argv[1], "r");
	if (!file) {
		fprintf(stderr, "Error opening file: %s\n", argv[1]);
		return 1;
	}
	u32 file_len = 0;

	fseek(file, 0, SEEK_END);
	file_len = ftell(file);
	fseek(file, 0, SEEK_SET);

	u8 *data = malloc(file_len);
	if (data == NULL) {
		fprintf(stderr, "Error allocating memory\n");
		return 1;
	}

	fread(data, file_len, 1, file);
	fclose(file);

	note_t *notes = malloc(sizeof(note_t) * (file_len / (sizeof(u32) * 2)));
	if (notes == NULL) {
		free(data);
		fprintf(stderr, "Error allocating memory\n");
		return 1;
	}
	if (file_len % (sizeof(u32) * 2) != 0) {
		free(data);
		free(notes);
		fprintf(stderr, "Invalid file format\n");
		return 1;
	}

	u32 p = 0;
	u32 notes_len = 0;
	while (p < file_len) {
		notes[notes_len].frequency = *(u32 *)(&data[p]);
		p += sizeof(u32);
		notes[notes_len].duration_ms = *(u32 *)(&data[p]);
		p += sizeof(u32);
		notes_len++;
	}

	for (u32 i = 0; i < notes_len; i++) {
		freq(notes[i].frequency);
		usleep(get_real_duration_ms(notes[i].duration_ms, 69) * 2);
		stop();
	}
	free(data);
	free(notes);
    return 0;
}

/*
file format:
	note1:
		u32 (little endian) frequency
		u32 (little endian) duration
	note2:
		u32 (little endian) frequency
		u32 (little endian) duration
	...
*/