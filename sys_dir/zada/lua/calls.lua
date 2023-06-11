local syscalls = {
    fs_get_element_name = 2,
    fs_make_dir = 3,
    fs_make_file = 4,
    fs_read_file = 5,
    fs_write_in_file = 6,
    fs_get_file_size = 7,
    fs_get_dir_size = 8,
    fs_does_path_exists = 10,
    fs_get_sector_type = 11,
    fs_get_dir_content = 12,
    fs_path_to_id = 13,
    malloc = 17,
    free = 18,
    clear = 24,
    serial_print = 43,
    process_sleep = 48,
    process_get = 51,
}

local function get_syscall(function_id)
    return profan.call_c(profan.memval(0x1ffff7, 4), 4, function_id)
end

local function get_lib_func(lib_id, func_id)
    return profan.call_c(profan.memval(0x1ffffb, 4), 4, lib_id, 4, func_id)
end

local function clear()
    profan.call_c(get_syscall(syscalls.clear))
end

local function malloc(size)
    return profan.call_c(get_syscall(syscalls.malloc), 4, size, 4, 1)
end

local function free(ptr)
    profan.call_c(get_syscall(syscalls.free), 4, ptr)
end

local function serial_print(str, serial_port)
    serial_port = serial_port or 0x3F8

    local ptr = malloc(#str + 1)

    -- Copy string to memory
    for i = 0, #str - 1 do
        profan.memset(ptr + i, 1, str:byte(i + 1))
    end
    profan.memset(ptr + #str, 1, 0)

    -- call serial_print
    profan.call_c(get_syscall(syscalls.serial_print), 4, serial_port, 4, ptr)

    free(ptr)
end

local function ms_sleep(ms)
    profan.call_c(get_syscall(syscalls.process_sleep), 4, profan.call_c(get_syscall(syscalls.process_get)), 4, ms)
end

return {
    get_syscall = get_syscall,
    get_lib_func = get_lib_func,
    serial_print = serial_print,
    clear = clear,
    malloc = malloc,
    free = free,
    ms_sleep = ms_sleep,

    syscalls = syscalls,
}
