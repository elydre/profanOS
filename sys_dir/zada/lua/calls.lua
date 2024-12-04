local function malloc(size)
    return profan.cfunc("malloc", 4, size)
end

local function free(ptr)
    profan.cfunc("free", 4, ptr)
end

local function serial_print(str)
    local ptr = malloc(#str + 1)

    -- Copy string to memory
    for i = 0, #str - 1 do
        profan.memset(ptr + i, 1, str:byte(i + 1))
    end
    profan.memset(ptr + #str, 1, 0)

    -- call serial_print
    profan.cfunc("syscall_serial_write", 4, 0x3F8, 4, ptr, 4, #str)

    free(ptr)
end

local function ms_sleep(ms)
    profan.cfunc("usleep", 4, ms * 1000)
end

return {
    malloc = malloc,
    free = free,
    serial_print = serial_print,
    ms_sleep = ms_sleep,
}
