/*****************************************************************************\
|   === interdisk.c : 2026 ===                                                |
|                                                                             |
|    Kernel global disk interface functions                        .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <kernel/butterfly.h>
#include <kernel/afft.h>
#include <minilib.h>
#include <system.h>

int g_disk_to_afft[SID_MAX_DISK];

int interdisk_init(void) {
    for (int i = 0; i < SID_MAX_DISK; i++)
        g_disk_to_afft[i] = -1;
    return 0;
}

int interdisk_register_disk(int disk, int afft_id) {
    if (disk >= SID_MAX_DISK || !afft_cmd(afft_id, AFFTC_EXISTS, NULL))
        return -1;

    if (disk >= 0) {
        g_disk_to_afft[disk] = afft_id;
        return disk;
    }

    for (int i = 0; i < SID_MAX_DISK; i++) {
        if (g_disk_to_afft[i] == -1) {
            g_disk_to_afft[i] = afft_id;
            return i;
        }
    }

    return -1;
}

int interdisk_write(sid_t sid, void *data, uint32_t size) {
    int afft = g_disk_to_afft[SID_DISK(sid)];
    if (afft == -1) {
        sys_error("Disk not registered");
        return 1;
    }

    return afft_write(afft, data, SID_SECTOR(sid) * SECTOR_SIZE, size) != (int) size;
}

int interdisk_read(sid_t sid, void *buffer, uint32_t size) {
    int afft = g_disk_to_afft[SID_DISK(sid)];
    if (afft == -1) {
        sys_error("Disk not registered");
        return 1;
    }

    return afft_read(afft, buffer, SID_SECTOR(sid) * SECTOR_SIZE, size) != (int) size;
}

int interdisk_write_offset(sid_t sid, void *data, uint32_t size, uint32_t offset) {
    int afft = g_disk_to_afft[SID_DISK(sid)];
    if (afft == -1) {
        sys_error("Disk not registered");
        return 1;
    }

    return afft_write(afft, data, SID_SECTOR(sid) * SECTOR_SIZE + offset, size) != (int) size;
}

int interdisk_read_offset(sid_t sid, void *buffer, uint32_t size, uint32_t offset) {
    int afft = g_disk_to_afft[SID_DISK(sid)];
    if (afft == -1) {
        sys_error("Disk not registered");
        return 1;
    }

    return afft_read(afft, buffer, SID_SECTOR(sid) * SECTOR_SIZE + offset, size) != (int) size;
}
