/*****************************************************************************\
|   === syscall.h : 2024 ===                                                  |
|                                                                             |
|    Userspace header for profanOS syscalls                        .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef _PROFAN_SYSCALL_H
#define _PROFAN_SYSCALL_H

#include <profan/minimal.h>
#include <profan/types.h>
#include <stdint.h>
#include <time.h> // struct tm

_BEGIN_C_FILE

/**********************************
 *                               *
 *  defines and macro functions  *
 *                               *
**********************************/

#define PROC_INFO_PPID     0
#define PROC_INFO_STATE    1
#define PROC_INFO_SLEEP_TO 2
#define PROC_INFO_RUN_TIME 3
#define PROC_INFO_NAME     4
#define PROC_INFO_STACK    5
#define PROC_INFO_SET_NAME 6

#define syscall_process_ppid(pid) syscall_process_info(pid, PROC_INFO_PPID, NULL)
#define syscall_process_state(pid) syscall_process_info(pid, PROC_INFO_STATE, NULL)
#define syscall_process_run_time(pid) syscall_process_info(pid, PROC_INFO_RUN_TIME, NULL)

#define syscall_kprint(message) syscall_kcnprint(message, -1, 0)

#define syscall_vesa_width()   syscall_vesa_info(0)
#define syscall_vesa_height()  syscall_vesa_info(1)
#define syscall_vesa_pitch()   syscall_vesa_info(2)
#define syscall_vesa_fb() ((void *) syscall_vesa_info(3))
#define syscall_vesa_state()   syscall_vesa_info(4)

#define syscall_timer_get_ms() syscall_ms_get() // retrocompatibility

/*************************************
 *                                  *
 *  syscall prototypes declaration  *
 *                                  *
*************************************/

filesys_t *syscall_fs_get_default(void);
int        syscall_fs_read(uint32_t, void *, uint32_t, uint32_t);
int        syscall_fs_write(uint32_t, void *, uint32_t, uint32_t);
int        syscall_fs_set_size(uint32_t, uint32_t);
uint32_t   syscall_fs_get_size(uint32_t);
int        syscall_fs_delete(uint32_t);
uint32_t   syscall_fs_init(uint32_t, char *);
char      *syscall_fs_meta(uint32_t, char *);

void    *syscall_mem_alloc(uint32_t, int, uint32_t);
int      syscall_mem_free(void *);
int      syscall_mem_free_all(int, int);
uint32_t syscall_mem_fetch(void *, int);
int      syscall_mem_info(int, int);

uint32_t syscall_ms_get(void);
int      syscall_time_get(struct tm *);

uint8_t *syscall_font_get(void);
int      syscall_kcnprint(char *, int, char);
int      syscall_vesa_info(int);

int      syscall_afft_read( int, void *, uint32_t, uint32_t);
int      syscall_afft_write(int, void *, uint32_t, uint32_t);
int      syscall_afft_cmd(  int, uint32_t, void *);

int      syscall_sc_get(void);

int      syscall_sys_power(int);
int      syscall_elf_exec(uint32_t, char **, char **);

int      syscall_process_create(void *, int, int, uint32_t *);
int      syscall_process_fork(void);
int      syscall_process_sleep(uint32_t, uint32_t);
int      syscall_process_wakeup(uint32_t, int);
int      syscall_process_wait(int, uint8_t *, int);
int      syscall_process_kill(uint32_t, int);
uint32_t syscall_process_pid(void);
int      syscall_process_info(uint32_t, int, void *);
int      syscall_process_list_all(uint32_t *, int);

int      syscall_mod_load(char *, uint32_t);
int      syscall_mod_unload(uint32_t);

void    *syscall_scuba_generate(void *, uint32_t);
int      syscall_scuba_map(void *, void *, int);
int      syscall_scuba_unmap(void *);
void    *syscall_scuba_phys(void *);

/*********************************
 *                              *
 *  syscalls declaration macro  *
 *                              *
*********************************/

extern int profan_syscall(uint32_t id, ...);

#define syscall_fs_get_default() ((filesys_t *) profan_syscall(0))
#define syscall_fs_read(a, b, c, d) ((int) profan_syscall(1, a, b, c, d))
#define syscall_fs_write(a, b, c, d) ((int) profan_syscall(2, a, b, c, d))
#define syscall_fs_set_size(a, b) ((int) profan_syscall(3, a, b))
#define syscall_fs_get_size(a) ((uint32_t) profan_syscall(4, a))
#define syscall_fs_delete(a) ((int) profan_syscall(5, a))
#define syscall_fs_init(a, b) ((uint32_t) profan_syscall(6, a, b))
#define syscall_fs_meta(a, b) ((char *) profan_syscall(7, a, b))

#define syscall_mem_alloc(a, b, c) ((void *) profan_syscall(8, a, b, c))
#define syscall_mem_free(a) ((int) profan_syscall(9, a))
#define syscall_mem_free_all(a, b) ((int) profan_syscall(10, a, b))
#define syscall_mem_fetch(a, b) ((uint32_t) profan_syscall(11, a, b))
#define syscall_mem_info(a, b) ((int) profan_syscall(12, a, b))

#define syscall_ms_get() ((uint32_t) profan_syscall(13))
#define syscall_time_get(a) ((int) profan_syscall(14, a))

#define syscall_font_get() ((uint8_t *) profan_syscall(15))
#define syscall_kcnprint(a, b, c) ((int) profan_syscall(16, a, b, c))
#define syscall_vesa_info(a) ((int) profan_syscall(17, a))

#define syscall_afft_read(a, b, c, d) ((int) profan_syscall(18, a, b, c, d))
#define syscall_afft_write(a, b, c, d) ((int) profan_syscall(19, a, b, c, d))
#define syscall_afft_cmd(a, b, c) ((int) profan_syscall(20, a, b, c))

#define syscall_sc_get() ((int) profan_syscall(21))

#define syscall_sys_power(a) ((int) profan_syscall(22, a))
#define syscall_elf_exec(a, b, c) ((int) profan_syscall(23, a, b, c))

#define syscall_process_create(a, b, c, d) ((int) profan_syscall(24, a, b, c, d))
#define syscall_process_fork() ((int) profan_syscall(25))
#define syscall_process_sleep(a, b) ((int) profan_syscall(26, a, b))
#define syscall_process_wakeup(a, b) ((int) profan_syscall(27, a, b))
#define syscall_process_wait(a, b, c) ((int) profan_syscall(28, a, b, c))
#define syscall_process_kill(a, b) ((int) profan_syscall(29, a, b))
#define syscall_process_pid() ((uint32_t) profan_syscall(30))
#define syscall_process_info(a, b, c) ((int) profan_syscall(31, a, b, c))
#define syscall_process_list_all(a, b) ((int) profan_syscall(32, a, b))

#define syscall_mod_load(a, b) ((int) profan_syscall(33, a, b))
#define syscall_mod_unload(a) ((int) profan_syscall(34, a))

#define syscall_scuba_generate(a, b) ((void *) profan_syscall(35, a, b))
#define syscall_scuba_map(a, b, c) ((int) profan_syscall(36, a, b, c))
#define syscall_scuba_unmap(a) ((int) profan_syscall(37, a))
#define syscall_scuba_phys(a) ((void *) profan_syscall(38, a))

_END_C_FILE

#endif
