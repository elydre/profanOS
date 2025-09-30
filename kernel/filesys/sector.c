/*****************************************************************************\
|   === sector.c : 2025 ===                                                   |
|                                                                             |
|    Part of the filesystem creation tool                          .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <kernel/butterfly.h>
#include <minilib.h>
#include <system.h>

int fs_sector_get_unused(int disk, int count, sid_t *ret) {
    static int next_free_sector = 1;
    *ret = SID_FORMAT(disk, next_free_sector);
    next_free_sector += count;
    return count;
}

int fs_sector_note_free(sid_t sid) {
    sys_error("not implemented yet\n");
    return 0;
}
