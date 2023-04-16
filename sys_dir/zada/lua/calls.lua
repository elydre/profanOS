function get_syscall(function_id)
    return profan.call_c(profan.memval(0x1ffff7, 4), 4, function_id)
end

function clear()
    profan.call_c(get_syscall(24))
end

function malloc(size)
    return profan.call_c(get_syscall(17), 4, size, 4, 1)
end

function free(ptr)
    profan.call_c(get_syscall(18), 4, ptr)
end

function serial_print(str, serial_port)
    serial_port = serial_port or 0x3F8

    local ptr = malloc(#str + 1)

    -- Copy string to memory
    for i = 0, #str - 1 do
        profan.memset(ptr + i, 1, str:byte(i + 1))
    end
    profan.memset(ptr + #str, 1, 0)

    -- call serial_print
    profan.call_c(get_syscall(43), 4, serial_port, 4, ptr)
    
    free(ptr)
end

return {
    get_syscall = get_syscall,
    serial_print = serial_print,
    clear = clear,
    malloc = malloc,
    free = free,
}
