#include <kernel/snowflake.h>
#include <kernel/butterfly.h>
#include <drivers/serial.h>
#include <kernel/process.h>
#include <minilib.h>
#include <system.h>
#include <ktype.h>

uint32_t **lib_functions = 0;

int dily_does_loaded(uint32_t lib_id) {
    return !(
        (lib_id < 1000             ||
         lib_id >= DILY_MAX + 1000 ||
         lib_functions == 0
        ) || (
         lib_functions[lib_id - 1000] == 0
        )
    );
}

int dily_load(char *path, uint32_t lib_id) {
    sid_t file = fu_path_to_sid(fs_get_main(), ROOT_SID, path);
    if (IS_NULL_SID(file) || !fu_is_file(fs_get_main(), file)) {
        sys_error("Lib file not found");
        return 1;
    }

    if (lib_id < 1000 || lib_id >= DILY_MAX + 1000) {
        sys_error("Invalid lib id");
        return 1;
    }

    if (dily_does_loaded(lib_id)) {
        sys_error("Library %d already loaded", lib_id);
        return 1;
    }

    // allocate the lib_functions array if it doesnt exist
    if (lib_functions == 0) {
        lib_functions = (void *) mem_alloc(DILY_MAX * sizeof(uint32_t *), 0, 5);    // 5: library
        mem_set((uint8_t *) lib_functions, 0, DILY_MAX * sizeof(uint32_t *));
    }

    uint32_t file_size = fs_cnt_get_size(fs_get_main(), file);
    uint8_t *binary_mem = (uint8_t *) mem_alloc(file_size + RUN_LIB_STACK, 0, 5);   // 5: library

    fs_cnt_read(fs_get_main(), file, binary_mem, 0, file_size);

    int function_count = 0;

    // count the number of functions
    for (uint32_t i = 0; i < file_size; i++) {
        if (binary_mem[i] == 0x55 && binary_mem[i + 1] == 0x89)
            function_count++;
    }

    // save the function addresses
    uint32_t *addr_list = (uint32_t *) mem_alloc((function_count + 1) * sizeof(uint32_t), 0, 5); // 5: library
    addr_list[0] = function_count;

    function_count = 1;
    for (uint32_t i = 0; i < file_size; i++) {
        if (binary_mem[i] == 0x55 && binary_mem[i + 1] == 0x89) {
            addr_list[function_count] = (uint32_t) &binary_mem[i];
            function_count++;
        }
    }

    // call the library entry point to initialize the library
    uint8_t *stack = binary_mem + file_size;
    mem_set((uint8_t *) stack, 0, RUN_LIB_STACK);
    uint32_t old_esp = 0;
    asm volatile("mov %%esp, %0" : "=r" (old_esp));
    asm volatile("mov %0, %%esp" :: "r" (stack + RUN_LIB_STACK - 0x80));

    ((void (*)(void)) addr_list[1])();

    asm volatile("mov %0, %%esp" :: "r" (old_esp));

    // save the address list
    lib_functions[lib_id - 1000] = addr_list;

    return 0;
}

int dily_unload(uint32_t lib_id) {
    if (!dily_does_loaded(lib_id)) {
        sys_error("Library %d not loaded", lib_id);
        return 1;
    }
    free(lib_functions[lib_id - 1000]);
    lib_functions[lib_id - 1000] = 0;

    return 0;
}

uint32_t dily_get_func(uint32_t lib_id, uint32_t func_id) {
    if (!dily_does_loaded(lib_id)) {
        sys_error("Library %d not loaded requested by pid %d", lib_id, process_get_pid());
        return 0;
    }

    uint32_t *addr_list = lib_functions[lib_id - 1000];
    if (func_id >= addr_list[0]) {
        sys_error("Function %d not found in library %d", func_id, lib_id);
        return 0;
    }

    return addr_list[func_id + 1];
}
