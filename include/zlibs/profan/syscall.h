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

#ifndef SYSCALL_H
#define SYSCALL_H

#include <profan/type.h>

// TEMPORARY

#define WATFUNC_ADDR 0x1ffff7


/**********************************
 *                               *
 *  defines and macro functions  *
 *                               *
**********************************/

#define SERIAL_PORT_A 0x3F8
#define SERIAL_PORT_B 0x2F8

#define PROCESS_INFO_PPID       0
#define PROCESS_INFO_PRIORITY   1
#define PROCESS_INFO_STATE      2
#define PROCESS_INFO_SLEEP_TO   3
#define PROCESS_INFO_RUN_TIME   4
#define PROCESS_INFO_EXIT_CODE  5
#define PROCESS_INFO_NAME       6

#define syscall_process_get_ppid(pid) syscall_process_get_info(pid, PROCESS_INFO_PPID)
#define syscall_process_get_state(pid) syscall_process_get_info(pid, PROCESS_INFO_STATE)
#define syscall_process_get_run_time(pid) syscall_process_get_info(pid, PROCESS_INFO_RUN_TIME)

#define syscall_kcprint(message, color) syscall_kcnprint(message, -1, color)
#define syscall_kprint(message) syscall_kcprint(message, 0x0F)

#define syscall_vesa_get_width()   syscall_vesa_get_info(0)
#define syscall_vesa_get_height()  syscall_vesa_get_info(1)
#define syscall_vesa_get_pitch()   syscall_vesa_get_info(2)
#define syscall_vesa_get_fb() (void *) syscall_vesa_get_info(3)
#define syscall_vesa_does_enable() syscall_vesa_get_info(4)

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

DEFN_SYSCALL0(0,  filesys_t *, fs_get_main)
DEFN_SYSCALL3(1,  int, fs_cnt_set_size, filesys_t *, uint32_t, uint32_t)
DEFN_SYSCALL2(2,  uint32_t, fs_cnt_get_size, filesys_t *, uint32_t)
DEFN_SYSCALL2(3,  int, fs_cnt_delete, filesys_t *, uint32_t)
DEFN_SYSCALL5(4,  int, fs_cnt_read, filesys_t *, uint32_t, void *, uint32_t, uint32_t)
DEFN_SYSCALL5(5,  int, fs_cnt_write, filesys_t *, uint32_t, void *, uint32_t, uint32_t)
DEFN_SYSCALL3(6,  uint32_t, fs_cnt_init, filesys_t *, uint32_t, char *)
DEFN_SYSCALL3(7,  char *, fs_cnt_meta, filesys_t *, uint32_t, char *)
DEFN_SYSCALL1(8,  uint32_t, mem_get_alloc_size, uint32_t)
DEFN_SYSCALL3(9,  uint32_t, mem_alloc, uint32_t, uint32_t, int)
DEFN_SYSCALL1(10, int, mem_free_addr, uint32_t)
DEFN_SYSCALL2(11, int, mem_get_info, int, int)
DEFN_SYSCALL1(12, int, mem_free_all, int)
DEFN_SYSCALL1(13, int, time_get, tm_t *) // no return value
DEFN_SYSCALL0(14, uint32_t, timer_get_ms)
DEFN_SYSCALL1(15, uint8_t *, font_get, int)
DEFN_SYSCALL3(16, int, kcnprint, char *, int, char) // no return value
DEFN_SYSCALL0(17, int, get_cursor_offset)
DEFN_SYSCALL3(18, int, vesa_set_pixel, int, int, uint32_t) // no return value
DEFN_SYSCALL1(19, int, vesa_get_info, int)
DEFN_SYSCALL1(20, int, sys_set_reporter, void *) // no return value
DEFN_SYSCALL2(21, char, kb_scancode_to_char, int, int)
DEFN_SYSCALL0(22, int, kb_get_scancode)
DEFN_SYSCALL0(23, int, kb_get_scfh)
DEFN_SYSCALL0(24, int, sys_reboot) // no return value
DEFN_SYSCALL0(25, int, sys_shutdown) // no return value
DEFN_SYSCALL4(26, int, binary_exec, uint32_t, int, char **, char **)
DEFN_SYSCALL0(27, char *, sys_kinfo)
DEFN_SYSCALL3(28, int, serial_read, int, char *, uint32_t) // no return value
DEFN_SYSCALL3(29, int, serial_write, int, char *, uint32_t) // no return value
DEFN_SYSCALL2(30, int, mouse_call, int, int)
DEFN_SYSCALL1(31, int, process_set_scheduler, int) // no return value
DEFN_SYSCALL5(32, int, process_create, void *, int, char *, int, uint32_t *)
DEFN_SYSCALL0(33, int, process_fork)
DEFN_SYSCALL2(34, int, process_sleep, uint32_t, uint32_t)
DEFN_SYSCALL1(35, int, process_handover, uint32_t)
DEFN_SYSCALL1(36, int, process_wakeup, uint32_t)
DEFN_SYSCALL1(37, int, process_kill, uint32_t) // no return value
DEFN_SYSCALL0(38, uint32_t, process_get_pid)
DEFN_SYSCALL2(39, int, process_generate_pid_list, uint32_t *, int)
DEFN_SYSCALL2(40, uint32_t, process_get_info, uint32_t, int)
DEFN_SYSCALL3(41, int, exit_pid, int, int, int)
DEFN_SYSCALL1(42, int, dily_unload, uint32_t)
DEFN_SYSCALL2(43, int, dily_load, char *, uint32_t)
DEFN_SYSCALL2(44, int, scuba_generate, void *, uint32_t) // no return value
DEFN_SYSCALL3(45, int, scuba_map, void *, void *, int) // no return value
DEFN_SYSCALL1(46, int, scuba_unmap, void *) // no return value
DEFN_SYSCALL1(47, void *, scuba_phys, void *)

#endif
