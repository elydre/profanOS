#include <kernel/snowflake.h>
#include <kernel/butterfly.h>
#include <drivers/serial.h>
#include <minilib.h>
#include <system.h>
#include <ktype.h>

uint32_t **lib_functions = 0;
int lib_count = 0;

int dily_does_loaded(int lib_id) {
    for (int i = 0; i < lib_count; i++) {
        if (lib_functions[i][0] != (uint32_t) lib_id)
            continue;
        return 1;
    }
    return 0;
}

int dily_load(char *path, int lib_id) {
    sid_t file = fu_path_to_sid(fs_get_main(), ROOT_SID, path);
    if (IS_NULL_SID(file) || !fu_is_file(fs_get_main(), file)) {
        sys_error("Lib file not found");
        return 1;
    }

    if (dily_does_loaded(lib_id)) {
        sys_error("Lib already loaded");
        return 1;
    }

    if (lib_functions == 0) {
        lib_functions = calloc(0x400);
    }

    uint32_t file_size = fs_cnt_get_size(fs_get_main(), file);
    uint8_t *binary_mem = (uint8_t *) mem_alloc(file_size + RUN_LIB_STACK, 0, 5); // 5: library

    fs_cnt_read(fs_get_main(), file, binary_mem, 0, file_size);

    uint32_t *addr_list = (uint32_t *) mem_alloc(0x800, 0, 5); // 6: as kernel
    addr_list[0] = (uint32_t) lib_id;

    int addr_list_size = 1;

    for (uint32_t i = 0; i < file_size; i++) {
        if (!(binary_mem[i] == 0x55 && binary_mem[i + 1] == 0x89))
            continue;
        addr_list[addr_list_size] = (uint32_t) &binary_mem[i];
        addr_list_size++;

        if (addr_list_size >= 512) {
            sys_fatal("Too many functions in library");
            return 1;
        }
    }

    uint8_t *stack = binary_mem + file_size;
    mem_set((uint8_t *) stack, 0, RUN_LIB_STACK);
    uint32_t old_esp = 0;
    asm volatile("mov %%esp, %0" : "=r" (old_esp));
    asm volatile("mov %0, %%esp" :: "r" (stack + RUN_LIB_STACK - 0x80));

    ((void (*)(void)) addr_list[1])();

    asm volatile("mov %0, %%esp" :: "r" (old_esp));

    lib_functions[lib_count] = addr_list;
    lib_count++;

    return 0;
}

int dily_unload(int lib_id) {
    for (int i = 0; i < lib_count; i++) {
        if (lib_functions[i][0] != (uint32_t) lib_id)
            continue;

        free(lib_functions[i]);
        lib_functions[i] = 0;
        return 0;
    }

    sys_error("Library not found");
    return 1;
}

int dily_get_func(int lib_id, int func_id) {
    for (int i = 0; i < lib_count; i++) {
        if (lib_functions[i][0] != (uint32_t) lib_id)
            continue;

        // the first value is the standard lib entry
        int val = (int) lib_functions[i][func_id + 1];

        if (!val) sys_error("Function not found");
        return val;
    }

    serial_kprintf("lib %d not found\n", lib_id);
    sys_fatal("Library not found");
    return 0;
}
