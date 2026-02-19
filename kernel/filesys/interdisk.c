#include <kernel/butterfly.h>

int interdisk_write(sid_t sid, void *data, uint32_t size) {
    return vdisk_write(data, size, SID_SECTOR(sid) * SECTOR_SIZE);
}

int interdisk_read(sid_t sid, void *buffer, uint32_t size) {
    return vdisk_read(buffer, size, SID_SECTOR(sid) * SECTOR_SIZE);
}

int interdisk_write_offset(sid_t sid, void *data, uint32_t size, uint32_t offset) {
    return vdisk_write(data, size, SID_SECTOR(sid) * SECTOR_SIZE + offset);
}

int interdisk_read_offset(sid_t sid, void *buffer, uint32_t size, uint32_t offset) {
    return vdisk_read(buffer, size, SID_SECTOR(sid) * SECTOR_SIZE + offset);
}
