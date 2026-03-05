/*****************************************************************************\
|   === vdisk.c : 2025 ===                                                    |
|                                                                             |
|    Part of the filesystem creation tool                          .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

#include "butterfly.h"

#define DISK_EXTEND_SIZE SECTOR_SIZE * 64

int fd = -1;

void vdisk_close(void) {
    if (fd == -1)
        return;

    close(fd);
    fd = -1;
}

void vdisk_new(char *path) {
    vdisk_close();
    
    fd = open(path, O_RDWR | O_CREAT | O_TRUNC, 0666);

    if (fd == -1) {
        perror("Failed to create vdisk");
        exit(EXIT_FAILURE);
    }    
}

void vdisk_open(char *path) {
    vdisk_close();
    
    fd = open(path, O_RDWR);

    if (fd == -1) {
        perror("Failed to open vdisk");
        exit(EXIT_FAILURE);
    }
}

int vdisk_write(void *data, uint32_t size, unsigned long offset) {
    if (fd == -1) {
        fprintf(stderr, "vdisk_write: No vdisk open\n");
        return -1;
    }

    if ((long) offset + size > lseek(fd, 0, SEEK_END)) {
        if (ftruncate(fd, offset + DISK_EXTEND_SIZE) == -1) {
            perror("vdisk_write: Failed to extend vdisk");
            return -1;
        }
    }

    if (lseek(fd, offset, SEEK_SET) == -1) {
        perror("vdisk_write: Failed to seek");
        return -1;
    }

    if (write(fd, data, size) != size) {
        perror("vdisk_write: Failed to write");
        return -1;
    }

    return 0;
}

int vdisk_read(void *buffer, uint32_t size, unsigned long offset) {
    if (fd == -1) {
        fprintf(stderr, "vdisk_read: No vdisk open\n");
        return -1;
    }

    if (lseek(fd, offset, SEEK_SET) == -1) {
        perror("vdisk_read: Failed to seek");
        return -1;
    }

    if (read(fd, buffer, size) != size) {
        perror("vdisk_read: Failed to read");
        return -1;
    }

    return 0;
}
