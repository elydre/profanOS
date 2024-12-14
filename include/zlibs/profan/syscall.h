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

#include <profan/types.h>
#include <stdint.h>
#include <time.h> // tm_t

/**********************************
 *                               *
 *  defines and macro functions  *
 *                               *
**********************************/

#define SERIAL_PORT_A 0x3F8
#define SERIAL_PORT_B 0x2F8

#define PROC_INFO_PPID     0
#define PROC_INFO_STATE    1
#define PROC_INFO_SLEEP_TO 2
#define PROC_INFO_RUN_TIME 3
#define PROC_INFO_NAME     4
#define PROC_INFO_STACK    5
#define PROC_INFO_COMM     6
#define PROC_INFO_SET_NAME 7

#define syscall_process_ppid(pid) syscall_process_info(pid, PROC_INFO_PPID, NULL)
#define syscall_process_state(pid) syscall_process_info(pid, PROC_INFO_STATE, NULL)
#define syscall_process_run_time(pid) syscall_process_info(pid, PROC_INFO_RUN_TIME, NULL)

#define syscall_kcprint(message, color) syscall_kcnprint(message, -1, color)
#define syscall_kprint(message) syscall_kcprint(message, 0x0F)

#define syscall_vesa_width()   syscall_vesa_info(0)
#define syscall_vesa_height()  syscall_vesa_info(1)
#define syscall_vesa_pitch()   syscall_vesa_info(2)
#define syscall_vesa_fb() ((void *) syscall_vesa_info(3))
#define syscall_vesa_state()   syscall_vesa_info(4)

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

#elif defined(_SYSCALL_CREATE_STATIC)

#define DEFN_SYSCALL0(id, ret_type, name) \
    static ret_type syscall_##name(void) { \
        ret_type a; \
        asm volatile( \
            "int $0x80" \
            : "=a" (a) \
            : "a" (id) \
        ); \
        return a; \
    }

#define DEFN_SYSCALL1(id, ret_type, name, type1) \
    static ret_type syscall_##name(type1 a1) { \
        ret_type a; \
        asm volatile( \
            "int $0x80" \
            : "=a" (a) \
            : "a" (id), "b" (a1) \
        ); \
        return a; \
    }

#define DEFN_SYSCALL2(id, ret_type, name, type1, type2) \
    static ret_type syscall_##name(type1 a1, type2 a2) { \
        ret_type a; \
        asm volatile( \
            "int $0x80" \
            : "=a" (a) \
            : "a" (id), "b" (a1), "c" (a2) \
        ); \
        return a; \
    }

#define DEFN_SYSCALL3(id, ret_type, name, type1, type2, type3) \
    static ret_type syscall_##name(type1 a1, type2 a2, type3 a3) { \
        ret_type a; \
        asm volatile( \
            "int $0x80" \
            : "=a" (a) \
            : "a" (id), "b" (a1), "c" (a2), "d" (a3) \
        ); \
        return a; \
    }

#define DEFN_SYSCALL4(id, ret_type, name, type1, type2, type3, type4) \
    static ret_type syscall_##name(type1 a1, type2 a2, type3 a3, type4 a4) { \
        ret_type a; \
        asm volatile( \
            "int $0x80" \
            : "=a" (a) \
            : "a" (id), "b" (a1), "c" (a2), "d" (a3), "S" (a4) \
        ); \
        return a; \
    }

#define DEFN_SYSCALL5(id, ret_type, name, type1, type2, type3, type4, type5) \
    static ret_type syscall_##name(type1 a1, type2 a2, type3 a3, type4 a4, type5 a5) { \
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
DEFN_SYSCALL5(1, int,        fs_read, filesys_t *, uint32_t, void *, uint32_t, uint32_t)
DEFN_SYSCALL5(2, int,        fs_write, filesys_t *, uint32_t, void *, uint32_t, uint32_t)
DEFN_SYSCALL3(3, int,        fs_set_size, filesys_t *, uint32_t, uint32_t)
DEFN_SYSCALL2(4, uint32_t,   fs_get_size, filesys_t *, uint32_t)
DEFN_SYSCALL2(5, int,        fs_delete, filesys_t *, uint32_t)
DEFN_SYSCALL3(6, uint32_t,   fs_init, filesys_t *, uint32_t, char *)
DEFN_SYSCALL3(7, char *,     fs_meta, filesys_t *, uint32_t, char *)

DEFN_SYSCALL3(8,  uint32_t,  mem_alloc, uint32_t, uint32_t, int)
DEFN_SYSCALL1(9,  int,       mem_free, uint32_t)
DEFN_SYSCALL1(10, int,       mem_free_all, int)
DEFN_SYSCALL1(11, uint32_t,  mem_get_alloc_size, uint32_t)
DEFN_SYSCALL2(12, int,       mem_info, int, int)

DEFN_SYSCALL0(13, uint32_t,  timer_get_ms)
DEFN_SYSCALL1(14, int,       time_get, tm_t *)

DEFN_SYSCALL0(15, uint8_t *, font_get)
DEFN_SYSCALL3(16, int,       kcnprint, char *, int, char)
DEFN_SYSCALL0(17, int,       get_cursor)
DEFN_SYSCALL1(18, int,       vesa_info, int)

DEFN_SYSCALL3(19, int,       serial_read, int, char *, uint32_t)
DEFN_SYSCALL3(20, int,       serial_write, int, char *, uint32_t)

DEFN_SYSCALL2(21, char,      sc_to_char, int, int)
DEFN_SYSCALL0(22, int,       sc_get)
DEFN_SYSCALL2(23, int,       mouse_call, int, int)

DEFN_SYSCALL1(24, int,       sys_set_reporter, void *)
DEFN_SYSCALL1(25, int,       sys_power, int)
DEFN_SYSCALL0(26, char *,    sys_kinfo)

DEFN_SYSCALL4(27, int,       elf_exec, uint32_t, int, char **, char **)

DEFN_SYSCALL1(28, int,       process_auto_schedule, int)
DEFN_SYSCALL4(29, int,       process_create, void *, int, int, uint32_t *)
DEFN_SYSCALL0(30, int,       process_fork)
DEFN_SYSCALL2(31, int,       process_sleep, uint32_t, uint32_t)
DEFN_SYSCALL2(32, int,       process_wakeup, uint32_t, int)
DEFN_SYSCALL3(33, int,       process_wait, int, uint8_t *, int)
DEFN_SYSCALL2(34, int,       process_kill, uint32_t, int)
DEFN_SYSCALL0(35, uint32_t,  process_pid)
DEFN_SYSCALL3(36, int,       process_info, uint32_t, int, void *)
DEFN_SYSCALL2(37, int,       process_list_all, uint32_t *, int)

DEFN_SYSCALL2(38, int,       pok_load, char *, uint32_t)
DEFN_SYSCALL1(39, int,       pok_unload, uint32_t)

DEFN_SYSCALL2(40, void *,    scuba_generate, void *, uint32_t)
DEFN_SYSCALL3(41, int,       scuba_map, void *, void *, int)
DEFN_SYSCALL1(42, int,       scuba_unmap, void *)
DEFN_SYSCALL1(43, void *,    scuba_phys, void *)

#endif
