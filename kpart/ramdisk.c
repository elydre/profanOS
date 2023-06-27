#include <driver/diskiso.h>
#include <driver/serial.h>
#include <driver/ata.h>
#include <gui/gnrtx.h>
#include <minilib.h>
#include <system.h>

#define UINT32_PER_SECTOR 128
#define RAMDISK_SIZE RAMDISK_SECTOR * UINT32_PER_SECTOR * 4

#define I_FILE_H 0x1
#define I_DIR    0x100
#define MAX_NAME 32

uint32_t ata_table[RAMDISK_SECTOR];
uint32_t *RAMDISK;
uint32_t ata_sector_count;
uint32_t diskiso_sector_count;
int table_pos = 0;

/* add in this list the paths 
to load in ramdisk at boot */

char *path_to_load[] = {
    "/bin/commands",
    "/bin/shell.bin",
};

/************************
 * READ & WRITE SECTOR *
************************/

int ramdisk_sector_internal_pos(uint32_t sector) {
    if (!ata_sector_count) {
        if (sector < RAMDISK_SECTOR) return sector;
    } else {
        for (int i = 0; i < table_pos; i++) {
            if (ata_table[i] == sector) return i;
        }
    }
    return -1;
}

void ramdisk_load_sector(uint32_t ATA_LBA, uint32_t in[]) {
    if (table_pos >= RAMDISK_SECTOR) {
        sys_fatal("No more space in ramdisk");
    }
    if (ramdisk_sector_internal_pos(ATA_LBA) != -1) return;
    ata_table[table_pos] = ATA_LBA;
    for (int i = 0; i < UINT32_PER_SECTOR; i++) {
        RAMDISK[(table_pos * UINT32_PER_SECTOR) + i] = in[i];
    }
    table_pos++;
}

void ramdisk_read_sector(uint32_t LBA, uint32_t out[]) {
    if (diskiso_sector_count) {
        diskiso_read(LBA, out);
        return;
    }

    int internal = ramdisk_sector_internal_pos(LBA);
    if (internal == -1) {
        if (!ata_sector_count) sys_error("Sector not found in ramdisk");
        else ata_read_sector(LBA, out);
        return;
    }
    for (int i = 0; i < UINT32_PER_SECTOR; i++) {
        out[i] = RAMDISK[internal * UINT32_PER_SECTOR + i];
    }
}

void ramdisk_write_sector(uint32_t sector, uint32_t* buffer) {
    if (diskiso_sector_count) {
        diskiso_write(sector, buffer);
        return;
    }

    if (ata_sector_count) {
        ata_write_sector(sector, buffer);
    }
    int internal = ramdisk_sector_internal_pos(sector);
    if (internal == -1) {
        if (!ata_sector_count) {
            sys_error("Sector not found in ramdisk");
        }
        return;
    }
    for (int i = 0; i < UINT32_PER_SECTOR; i++) {
        RAMDISK[internal * UINT32_PER_SECTOR + i] = buffer[i];
    }
}

/***************************
 * RAMDISK INITIALIZATION *
***************************/

void ramdisk_check_dir(char parent_name[], uint32_t sector_id);

int ramdisk_init() {
    diskiso_sector_count = diskiso_get_size();
    if (diskiso_sector_count) {
        sys_warning("Using diskiso as ramdisk");
        ata_sector_count = 0;
        return 0;
    }
    
    RAMDISK = malloc(RAMDISK_SIZE);

    ata_sector_count = ata_get_sectors_count();
    if (ata_sector_count) {
        char path[256];
        for (int i = 0; i < 256; i++) path[i] = 0;
        ramdisk_check_dir(path, 0);
    } else {
        sys_warning("ATA disk not working");
    }
    return 0;
}

void load_file(uint32_t first_sector_id) {
    uint32_t sector[UINT32_PER_SECTOR];
    uint32_t sector_id = first_sector_id;
    do {
        ata_read_sector(sector_id, sector);
        ramdisk_load_sector(sector_id, sector);
        sector_id = sector[UINT32_PER_SECTOR - 1];
    } while (sector[UINT32_PER_SECTOR - 1] != 0);
}

int str_atb(char str[], char begin[]) {
    // str at begin
    for (int i = 0; i < str_len(begin); i++) {
        if (str[i] != begin[i]) return 0;
    }
    return 1;
}

void ramdisk_check_dir(char parent_name[], uint32_t sector_id) {
    if (ramdisk_sector_internal_pos(sector_id) != -1) return;

    if (ata_sector_count < sector_id) {
        sys_error("Sector can't exist");
        return;
    }

    char fullname[256];
    for (int i = 0; i < 256; i++) fullname[i] = parent_name[i];
    uint32_t sector[UINT32_PER_SECTOR];
    ata_read_sector(sector_id, sector);

    if (!(sector[0] & (I_FILE_H | I_DIR))) {
        kprintf("FATAL: %x in sec %d\n", sector[0], sector_id);
        sys_fatal("dametokosita find in dir");
    }

    char name[MAX_NAME];
    for (int i = 0; i < MAX_NAME; i++) name[i] = sector[i + 1];
    name[MAX_NAME - 1] = 0;
    if (fullname[1] != 0 && fullname[0] != 0) str_append(fullname, '/');
    str_cat(fullname, name);

    if (sector[0] & I_FILE_H) {
        for (int i = 0; i < ARYLEN(path_to_load); i++) {
            if (str_atb(fullname, path_to_load[i]) || str_cmp(fullname, path_to_load[i]) == 0) {
                serial_debug("RD-LF", fullname);
                load_file(sector_id);
            }
        }
        return;
    }

    // check if the path is the beginning of a path to load
    for (int i = 0; i < ARYLEN(path_to_load); i++) {
        if (str_atb(fullname, path_to_load[i]) || str_atb(path_to_load[i], fullname) || str_cmp(fullname, path_to_load[i]) == 0) {
            ramdisk_load_sector(sector_id, sector);
            // TODO: dir continue gestion
            for (int i = MAX_NAME + 1; i < UINT32_PER_SECTOR - 1; i++) {
                if (sector[i] == 0) continue;
                ramdisk_check_dir(fullname, sector[i]);
            }
        }
    }
}

/***************
 * GET & INFO *
***************/

uint32_t ramdisk_get_address() {
    return (uint32_t) RAMDISK;
}

uint32_t ramdisk_get_info(int info) {
    if (info == 0) return RAMDISK_SECTOR;
    if (info == 1) return table_pos;
    return 0;
}
