#include <kernel/filesystem.h>
#include <kernel/snowflake.h>
#include <driver/serial.h>
#include <minilib.h>
#include <system.h>
#include <type.h>

uint32_t **lib_functions = 0;
int lib_count = 0;

typedef struct {
    int id;
    char path[256];
} lib_t;

lib_t libs_at_boot[] = {
    {1000, "/lib/i_iolib.bin"},
    {1001, "/lib/i_string.bin"},
    {1002, "/lib/profan.bin"},
    {1003, "/lib/i_mem.bin"},
    {1004, "/lib/i_time.bin"},
    {1006, "/lib/i_vgui.bin"},
    {1007, "/lib/stdlib.bin"},
    {1008, "/lib/string.bin"},
    {1009, "/lib/stdio.bin"},
    {1010, "/lib/i_mouse.bin"},
    {1011, "/lib/math.bin"},
    {1012, "/lib/time.bin"},
    {1013, "/lib/setjmp.bin"},
    {1014, "/lib/unistd.bin"},
    {1015, "/lib/i_libdaube.bin"},
};


int dily_does_loaded(int lib_id) {
    for (int i = 0; i < lib_count; i++) {
        if (lib_functions[i][0] != (uint32_t) lib_id) 
            continue;

        return 1;
    }
    return 0;
}

int dily_load(char path[], int lib_id) {
    if ((!fs_does_path_exists(path)) || fs_get_sector_type(fs_path_to_id(path)) != 2) {
        sys_error("Lib file not found");
        return 1;
    }

    if (dily_does_loaded(lib_id)) {
        sys_error("Lib already loaded");
        return 1;
    }

    if (lib_functions == 0) {
        lib_functions = calloc(0x400);
        // can be realloc in the future
    }

    int file_size = fs_get_file_size(path);
    int lib_size = file_size + RUN_LIB_STACK_L + RUN_LIB_STACK_R;
    uint8_t *binary_mem = (uint8_t *) mem_alloc(lib_size, 5); // 5 = library
    uint8_t *file = binary_mem + RUN_LIB_STACK_L;

    fs_read_file(path, (char *) file);

    uint32_t *addr_list = calloc(0x800);
    addr_list[0] = (uint32_t) lib_id;

    int addr_list_size = 1;

    for (int i = 0; i < file_size; i++) {
        if (file[i] == 0x55 && file[i + 1] == 0x89) {
            addr_list[addr_list_size] = (uint32_t) &file[i];
            addr_list_size++;
        }
    }

    ((void (*)(void)) addr_list[1])();

    lib_functions[lib_count] = addr_list;
    lib_count++;

    return 0;
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

    sys_fatal("Library not found");
    return 0;
}

void dily_unload(int lib_id) {
    for (int i = 0; i < lib_count; i++) {
        if (lib_functions[i][0] != (uint32_t) lib_id) 
            continue;

        free(lib_functions[i]);
        lib_functions[i] = 0;
        return;
    }

    sys_error("Library not found");
}

int dily_init() {
    int error_count = 0;
    for (int i = 0; i < (int) (sizeof(libs_at_boot) / sizeof(lib_t)); i++) {
        error_count += dily_load(libs_at_boot[i].path, libs_at_boot[i].id);
    }
    return error_count != 0;
}
