#include <kernel/snowflake.h>
#include <kernel/butterfly.h>
#include <drivers/serial.h>
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
    {1000, "/lib/profan/iolib.bin"},
    {1002, "/lib/profan/profan.bin"},
    {1004, "/lib/profan/itime.bin"},
    {1006, "/lib/profan/vgui.bin"},
    {1007, "/lib/ports/stdlib.bin"},
    {1008, "/lib/ports/string.bin"},
    {1009, "/lib/ports/stdio.bin"},
    {1010, "/lib/profan/filesys.bin"},
    {1011, "/lib/ports/math.bin"},
    {1012, "/lib/ports/time.bin"},
    {1013, "/lib/ports/setjmp.bin"},
    {1014, "/lib/ports/unistd.bin"},
};


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
        // can be realloc in the future
    }

    uint32_t file_size = fs_cnt_get_size(fs_get_main(), file);
    uint32_t lib_size = file_size + RUN_LIB_STACK_L + RUN_LIB_STACK_R;
    uint8_t *binary_mem = (uint8_t *) mem_alloc(lib_size, 0, 5); // 5: library
    uint8_t *cnt = binary_mem + RUN_LIB_STACK_L;

    // fs_read_file(path, (char *) file);
    fs_cnt_read(fs_get_main(), file, cnt, 0, file_size);

    uint32_t *addr_list = (uint32_t *) mem_alloc(0x800, 0, 5); // 6: as kernel
    addr_list[0] = (uint32_t) lib_id;

    int addr_list_size = 1;

    for (uint32_t i = 0; i < file_size; i++) {
        if (cnt[i] == 0x55 && cnt[i + 1] == 0x89) {
            addr_list[addr_list_size] = (uint32_t) &cnt[i];
            addr_list_size++;
        }
    }

    ((void (*)(void)) addr_list[1])();

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

    sys_fatal("Library not found");
    return 0;
}

int dily_init() {
    int error_count = 0;
    for (int i = 0; i < (int) (sizeof(libs_at_boot) / sizeof(lib_t)); i++) {
        error_count += dily_load(libs_at_boot[i].path, libs_at_boot[i].id);
    }
    return error_count != 0;
}