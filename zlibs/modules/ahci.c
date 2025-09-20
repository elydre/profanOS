/*****************************************************************************\
|   === ahci.c : 2025 ===                                                     |
|                                                                             |
|    -                                                             .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <minilib.h>
#include "ahci.h"

#include <kernel/snowflake.h>
#include <kernel/scubasuit.h>
#include <kernel/process.h>

#define get_phys_addr(x) (void *) scuba_get_phys(process_get_dir(process_get_pid()), x)
#define alloc_page(x) (void *) mem_alloc(4096 * (x), SNOW_KERNEL, 0x1000)


static void *calloc_page(int num) {
    void *ptr = alloc_page(num);
    mem_set(ptr, 0, 4096 * num);
    return ptr;
}

/* Error codes:
 *
 * 1 : No available slot
 * 2 : Port hung
 * 3 : Disk error
 * 4 : Drive not found
 */

typedef struct {
    HBAData *abar;
    HBAPort *port;
    void    *clb;
    void    *fb;
    void    *ctba[32];
    void    *unused[28]; // Even out the data size to 256 bytes
} ahci_port;

ahci_port *ports;
uint32_t   num_ports = 0;

static uint32_t find_cmdslot(ahci_port aport) {
    HBAPort *port = aport.port;
    uint32_t slots = (port->sact | port->ci);
    uint32_t cmdslots = (aport.abar->cap & 0x0f00) >> 8;
    for (uint32_t i = 0; i < cmdslots; i++) {
        if ((slots & 1) == 0)
            return i;
        slots >>= 1;
    }
    return 0xFFFFFFFF;
}

static uint8_t ahci_identify_device(ahci_port aport, void *buf) {
    HBAPort *port = aport.port;
    port->is = 0xFFFFFFFF;
    uint32_t slot = find_cmdslot(aport);
    if (slot == 0xFFFFFFFF)
        return 1;

    HBACommandHeader *cmdheader = (HBACommandHeader *)aport.clb;
    cmdheader += slot;
    cmdheader->cfl = sizeof(FIS_HostToDevice) / sizeof(uint32_t);
    cmdheader->w = 0;

    HBACommandTable *cmdtable = (HBACommandTable *)aport.ctba[slot];

    cmdtable->prdt_entry[0].dba = (uint32_t) get_phys_addr(buf);
    cmdtable->prdt_entry[0].dbau = 0;
    cmdtable->prdt_entry[0].dbc = 511;
    cmdtable->prdt_entry[0].i = 1;

    FIS_HostToDevice *cmdfis = (FIS_HostToDevice *)(&cmdtable->cfis);

    cmdfis->FIS_Type = 0x27; // Host to device
    cmdfis->c = 1;
    cmdfis->command = SATA_IDENTIFY_DEVICE;

    cmdfis->dev = 1 << 6;

    for (uint32_t spin = 0; spin < 1000000; spin++) {
        if (!(port->tfd & (SATA_BUSY | SATA_DRQ)))
            break;
    }
    if ((port->tfd & (SATA_BUSY | SATA_DRQ)))
        return 2;

    port->ci = (1 << slot);

    while (1) {
        if (!(port->ci & (1 << slot)))
            break;
        if (port->is & (1 << 30))
            return 3;
    }

    if (port->is & (1 << 30))
        return 3;

    return 0;
}

static uint8_t ahci_read_sectors_internal(ahci_port aport, uint32_t startl, uint32_t starth, uint32_t count, uint8_t *buf) {
    HBAPort *port = aport.port;
    port->is = 0xFFFFFFFF;
    uint32_t slot = find_cmdslot(aport);
    if (slot == 0xFFFFFFFF)
        return 1;

    HBACommandHeader *cmdheader = (HBACommandHeader *)aport.clb;
    cmdheader += slot;
    cmdheader->cfl = sizeof(FIS_HostToDevice) / sizeof(uint32_t);
    cmdheader->w = 0;

    HBACommandTable *cmdtable = (HBACommandTable *)aport.ctba[slot];

    cmdtable->prdt_entry[0].dba = (uint32_t)get_phys_addr(buf);
    cmdtable->prdt_entry[0].dbau = 0;
    cmdtable->prdt_entry[0].dbc = (count * 512) - 1;
    cmdtable->prdt_entry[0].i = 1;

    FIS_HostToDevice *cmdfis = (FIS_HostToDevice *)(&cmdtable->cfis);

    cmdfis->FIS_Type = 0x27; // Host to device
    cmdfis->c = 1;
    cmdfis->command = SATA_READ_DMA_EX;

    cmdfis->lba0 = (uint8_t)startl;
    cmdfis->lba1 = (uint8_t)(startl >> 8);
    cmdfis->lba2 = (uint8_t)(startl >> 16);
    cmdfis->dev = 1 << 6;

    cmdfis->lba3 = (uint8_t)(startl >> 24);
    cmdfis->lba4 = (uint8_t)(starth);
    cmdfis->lba5 = (uint8_t)(starth >> 8);

    cmdfis->countl = (count & 0xFF);
    cmdfis->counth = (count >> 8);

    for (uint32_t spin = 0; spin < 1000000; spin++) {
        if (!(port->tfd & (SATA_BUSY | SATA_DRQ)))
            break;
    }
    if ((port->tfd & (SATA_BUSY | SATA_DRQ)))
        return 2;

    port->ci = (1 << slot);

    while (1) {
        if (!(port->ci & (1 << slot)))
            break;
        if (port->is & (1 << 30))
            return 3;
    }

    if (port->is & (1 << 30))
        return 3;

    return 0;
}

static uint8_t ahci_write_sectors_internal(ahci_port aport, uint32_t startl, uint32_t starth, uint32_t count, uint8_t *buf) {
    HBAPort *port = aport.port;
    port->is = 0xFFFFFFFF;
    uint32_t slot = find_cmdslot(aport);
    if (slot == 0xFFFFFFFF)
        return 1;

    HBACommandHeader *cmdheader = (HBACommandHeader *)aport.clb;
    cmdheader += slot;
    cmdheader->cfl = sizeof(FIS_HostToDevice) / sizeof(uint32_t);
    cmdheader->w = 1; // We are writing this time

    HBACommandTable *cmdtable = (HBACommandTable *)aport.ctba[slot];

    cmdtable->prdt_entry[0].dba = (uint32_t)get_phys_addr(buf);
    cmdtable->prdt_entry[0].dbau = 0;
    cmdtable->prdt_entry[0].dbc = (count * 512) - 1;
    cmdtable->prdt_entry[0].i = 1;

    FIS_HostToDevice *cmdfis = (FIS_HostToDevice *)(&cmdtable->cfis);

    cmdfis->FIS_Type = 0x27; // Host to device
    cmdfis->c = 1;
    cmdfis->command = SATA_WRITE_DMA_EX;

    cmdfis->lba0 = (uint8_t)startl;
    cmdfis->lba1 = (uint8_t)(startl >> 8);
    cmdfis->lba2 = (uint8_t)(startl >> 16);
    cmdfis->dev = 1 << 6;

    cmdfis->lba3 = (uint8_t)(startl >> 24);
    cmdfis->lba4 = (uint8_t)(starth);
    cmdfis->lba5 = (uint8_t)(starth >> 8);

    cmdfis->countl = (count & 0xFF);
    cmdfis->counth = (count >> 8);

    for (uint32_t spin = 0; spin < 1000000; spin++) {
        if (!(port->tfd & (SATA_BUSY | SATA_DRQ)))
            break;
    }
    if ((port->tfd & (SATA_BUSY | SATA_DRQ)))
        return 2;

    port->ci = (1 << slot);

    while (1) {
        if (!(port->ci & (1 << slot)))
            break;
        if (port->is & (1 << 30))
            return 3;
    }

    if (port->is & (1 << 30))
        return 3;

    return 0;
}

static void initialize_port(ahci_port *aport) {
    HBAPort *port = aport->port;
    port->cmd &= ~HBA_CMD_ST;
    port->cmd &= ~HBA_CMD_FRE;

    while ((port->cmd & HBA_CMD_FR) || (port->cmd & HBA_CMD_CR))
        ;

    void *mapped_clb = alloc_page(1);
    mem_set(mapped_clb, 0, 4096);
    port->clb = (uint32_t)get_phys_addr(mapped_clb);
    port->clbu = 0;
    aport->clb = mapped_clb;

    void *mapped_fb = alloc_page(1);
    mem_set(mapped_fb, 0, 4096);
    port->fb = (uint32_t)get_phys_addr(mapped_fb);
    port->fbu = 0;
    aport->fb = mapped_fb;

    HBACommandHeader *cmdheader = (HBACommandHeader *)mapped_clb;

    for (uint8_t i = 0; i < 32; i++) {
        cmdheader[i].prdtl = 1;
        void *ctba_buf = calloc_page(1);
        aport->ctba[i] = ctba_buf;
        cmdheader[i].ctba = (uint32_t)get_phys_addr(ctba_buf);
        cmdheader[i].ctbau = 0;
    }

    while (port->cmd & HBA_CMD_CR)
        ;
    port->cmd |= HBA_CMD_FRE;
    port->cmd |= HBA_CMD_ST;
}

static int is_sata(HBAPort *port) {
    uint8_t ipm = (port->ssts >> 8) & 0xF;
    uint8_t det = (port->ssts) & 0xF;

    return (det == HBA_DET_PRESENT && ipm == HBA_IPM_ACTIVE);
}

static void initialize_abar(HBAData *abar) {
    uint32_t pi = abar->pi;

    for (uint8_t i = 0; i < 32; i++) {
        if (pi & 1) {
            // kprintf("[AHCI] Found port %x\n", i);

            if (is_sata(&abar->ports[i])) {
                mem_set(&ports[num_ports], 0, 256);
                ports[num_ports].abar = abar;
                ports[num_ports].port = &abar->ports[i];


                initialize_port(&ports[num_ports]);

                sata_identify_packet *info = calloc_page(1);
                kprintf("\n[AHCI] Identifying device: %d (0 = OK)\n", ahci_identify_device(ports[num_ports], info));

                char name[41] = {0};
                for (int i = 0; i < 40; i += 2) {
                    name[i] = info->model_number[i + 1];
                    name[i + 1] = info->model_number[i];
                }

                kprintf("[AHCI] Detected SATA drive: %s\n", name);
                kprintf("size: %d sector\n", (uint32_t) info->total_sectors & 0xFFFFFFFF);

                free(info);

                num_ports++;
            }
        }
        pi >>= 1;
    }
}

#define MAGIC_ADDR 0xFEBB1000
// #define MAGIC_ADDR 0xA132d000

uint8_t ahci_read_sectors(uint16_t drive_num, uint64_t start_sector, uint32_t count, void *buf) {
    scuba_call_map((void *) MAGIC_ADDR, (void *) MAGIC_ADDR, 1);
    kprintf_serial("ahci_read_sectors(drive_num=%d, start_sector=%x - %x, count=%d, buf=%x)\n", drive_num, (uint32_t)(start_sector & 0xFFFFFFFF), (uint32_t)((start_sector >> 32) & 0xFFFFFFFF), count, (uint32_t)buf);

    if (ports[drive_num].abar != 0)
        return ahci_read_sectors_internal(ports[drive_num], start_sector & 0xFFFFFFFF, (start_sector >> 32) & 0xFFFFFFFF, count, buf);
    else
        return 4;
}

uint8_t ahci_write_sectors(uint16_t drive_num, uint64_t start_sector, uint32_t count, void *buf) {
    scuba_call_map((void *) MAGIC_ADDR, (void *) MAGIC_ADDR, 1);

    if (ports[drive_num].abar != 0)
        return ahci_write_sectors_internal(ports[drive_num], start_sector & 0xFFFFFFFF, (start_sector >> 32) & 0xFFFFFFFF, count, buf);
    else
        return 4;
}

int drive_exists(uint16_t drive_num) {
    return drive_num < 256 && (ports[drive_num].abar != 0);
}

int __init(void) {
    ports = alloc_page(16); // 256*256 = 65536 bytes, or 16 pages. 256 ports is probably enough

    // register_pci_driver(print_pci_data, 1, 6);
    // driver_id = register_disk_handler(ahci_read_sectors, ahci_write_sectors, 255);

    scuba_call_map((void *) MAGIC_ADDR, (void *) MAGIC_ADDR, 1);

    initialize_abar((HBAData *) MAGIC_ADDR);

    return 0;
}
