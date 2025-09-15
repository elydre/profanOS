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

#define syscall_kprint(message) syscall_kcnprint(message, -1, 0x0F)

#define syscall_vesa_width()   syscall_vesa_info(0)
#define syscall_vesa_height()  syscall_vesa_info(1)
#define syscall_vesa_pitch()   syscall_vesa_info(2)
#define syscall_vesa_fb() ((void *) syscall_vesa_info(3))
#define syscall_vesa_state()   syscall_vesa_info(4)

#define syscall_timer_get_ms() syscall_ms_get() // retrocompatibility

/************************************
 *                                 *
 *  syscall generator definitions  *
 *                                 *
************************************/

#undef DEFN_SYSCALL0
#undef DEFN_SYSCALL1
#undef DEFN_SYSCALL2
#undef DEFN_SYSCALL3
#undef DEFN_SYSCALL4
#undef DEFN_SYSCALL5

#ifdef _SYSCALL_CREATE_FUNCS

#define DEFN_SYSCALL0(id, ret_type, name) \
    ret_type syscall_##name(void) { \
        ret_type a; \
        asm volatile( \
            "int $0x80" \
            : "=a" (a) \
            : "a" (id) \
        ); \
        return a; \
    }

#define DEFN_SYSCALL1(id, ret_type, name, type1) \
    ret_type syscall_##name(type1 a1) { \
        ret_type a; \
        asm volatile( \
            "int $0x80" \
            : "=a" (a) \
            : "a" (id), "b" (a1) \
        ); \
        return a; \
    }

#define DEFN_SYSCALL2(id, ret_type, name, type1, type2) \
    ret_type syscall_##name(type1 a1, type2 a2) { \
        ret_type a; \
        asm volatile( \
            "int $0x80" \
            : "=a" (a) \
            : "a" (id), "b" (a1), "c" (a2) \
        ); \
        return a; \
    }

#define DEFN_SYSCALL3(id, ret_type, name, type1, type2, type3) \
    ret_type syscall_##name(type1 a1, type2 a2, type3 a3) { \
        ret_type a; \
        asm volatile( \
            "int $0x80" \
            : "=a" (a) \
            : "a" (id), "b" (a1), "c" (a2), "d" (a3) \
        ); \
        return a; \
    }

#define DEFN_SYSCALL4(id, ret_type, name, type1, type2, type3, type4) \
    ret_type syscall_##name(type1 a1, type2 a2, type3 a3, type4 a4) { \
        ret_type a; \
        asm volatile( \
            "int $0x80" \
            : "=a" (a) \
            : "a" (id), "b" (a1), "c" (a2), "d" (a3), "S" (a4) \
        ); \
        return a; \
    }

#define DEFN_SYSCALL5(id, ret_type, name, type1, type2, type3, type4, type5) \
    ret_type syscall_##name(type1 a1, type2 a2, type3 a3, type4 a4, type5 a5) { \
        ret_type a; \
        asm volatile( \
            "int $0x80" \
            : "=a" (a) \
            : "a" (id), "b" (a1), "c" (a2), "d" (a3), "S" (a4), "D" (a5) \
        ); \
        return a; \
    }

#else

#define DEFN_SYSCALL0(id, ret_type, name) \
    ret_type syscall_##name(void);

#define DEFN_SYSCALL1(id, ret_type, name, type1) \
    ret_type syscall_##name(type1 a1);

#define DEFN_SYSCALL2(id, ret_type, name, type1, type2) \
    ret_type syscall_##name(type1 a1, type2 a2);

#define DEFN_SYSCALL3(id, ret_type, name, type1, type2, type3) \
    ret_type syscall_##name(type1 a1, type2 a2, type3 a3);

#define DEFN_SYSCALL4(id, ret_type, name, type1, type2, type3, type4) \
    ret_type syscall_##name(type1 a1, type2 a2, type3 a3, type4 a4);

#define DEFN_SYSCALL5(id, ret_type, name, type1, type2, type3, type4, type5) \
    ret_type syscall_##name(type1 a1, type2 a2, type3 a3, type4 a4, type5 a5);

#endif

/************************************
 *                                 *
 *  syscall function definitions   *
 *                                 *
************************************/

DEFN_SYSCALL0(0, filesys_t *,fs_get_default)
DEFN_SYSCALL4(1, int,        fs_read, uint32_t, void *, uint32_t, uint32_t)
DEFN_SYSCALL4(2, int,        fs_write, uint32_t, void *, uint32_t, uint32_t)
DEFN_SYSCALL2(3, int,        fs_set_size, uint32_t, uint32_t)
DEFN_SYSCALL1(4, uint32_t,   fs_get_size, uint32_t)
DEFN_SYSCALL1(5, int,        fs_delete, uint32_t)
DEFN_SYSCALL2(6, uint32_t,   fs_init, uint32_t, char *)
DEFN_SYSCALL2(7, char *,     fs_meta, uint32_t, char *)

DEFN_SYSCALL3(8,  void *,    mem_alloc, uint32_t, int, uint32_t)
DEFN_SYSCALL1(9,  int,       mem_free, void *)
DEFN_SYSCALL2(10, int,       mem_free_all, int, int)
DEFN_SYSCALL2(11, uint32_t,  mem_fetch, void *, int)
DEFN_SYSCALL2(12, int,       mem_info, int, int)

DEFN_SYSCALL0(13, uint32_t,  ms_get)
DEFN_SYSCALL1(14, int,       time_get, struct tm *)

DEFN_SYSCALL0(15, uint8_t *, font_get)
DEFN_SYSCALL3(16, int,       kcnprint, char *, int, char)
DEFN_SYSCALL1(17, int,       vesa_info, int)

DEFN_SYSCALL4(18, int,       afft_read,  int, void *, uint32_t, uint32_t)
DEFN_SYSCALL4(19, int,       afft_write, int, void *, uint32_t, uint32_t)
DEFN_SYSCALL3(20, int,       afft_cmd,   int, uint32_t, void *)

DEFN_SYSCALL0(21, int,       sc_get)

DEFN_SYSCALL1(22, int,       sys_power, int)
DEFN_SYSCALL3(23, int,       elf_exec, uint32_t, char **, char **)

DEFN_SYSCALL4(24, int,       process_create, void *, int, int, uint32_t *)
DEFN_SYSCALL0(25, int,       process_fork)
DEFN_SYSCALL2(26, int,       process_sleep, uint32_t, uint32_t)
DEFN_SYSCALL2(27, int,       process_wakeup, uint32_t, int)
DEFN_SYSCALL3(28, int,       process_wait, int, uint8_t *, int)
DEFN_SYSCALL2(29, int,       process_kill, uint32_t, int)
DEFN_SYSCALL0(30, uint32_t,  process_pid)
DEFN_SYSCALL3(31, int,       process_info, uint32_t, int, void *)
DEFN_SYSCALL2(32, int,       process_list_all, uint32_t *, int)

DEFN_SYSCALL2(33, int,       mod_load, char *, uint32_t)
DEFN_SYSCALL1(34, int,       mod_unload, uint32_t)

DEFN_SYSCALL2(35, void *,    scuba_generate, void *, uint32_t)
DEFN_SYSCALL3(36, int,       scuba_map, void *, void *, int)
DEFN_SYSCALL1(37, int,       scuba_unmap, void *)
DEFN_SYSCALL1(38, void *,    scuba_phys, void *)

_END_C_FILE

#endif
