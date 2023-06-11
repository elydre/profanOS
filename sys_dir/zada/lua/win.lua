local calls = require("calls")

local winfuncs = {
    window_create = 4,
    desktop_refresh = 5,

    window_set_pixel = 8,
    window_display_pixel = 9,
    window_refresh = 10,

    desktop_get_main = 12,
    window_delete = 13,
}

local LIBDAUBE_ID = 1015

------------------------
-- Internal functions --
------------------------

local function copy_string_to_memory(str)
    local ptr = calls.malloc(#str + 1)

    -- Copy string to memory
    for i = 0, #str - 1 do
        profan.memset(ptr + i, 1, str:byte(i + 1))
    end
    profan.memset(ptr + #str, 1, 0)

    return ptr
end

local function copy_memory_to_string(ptr, size)
    local str = ""

    for i = 0, size - 1 do
        local char = profan.memval(ptr + i, 1)
        if char == 0 then
            break
        end
        str = str .. string.char(char)
    end

    return str
end

------------------------
--  Shared functions  --
------------------------

local function desktop_get_main()
    return profan.call_c(calls.get_lib_func(LIBDAUBE_ID, winfuncs.desktop_get_main))
end

local function window_create(width, height, title, x, y, is_lite, cant_move, always_on_top)
    if not x then x = 100 end
    if not y then y = 100 end

    if is_lite then is_lite = 1 else is_lite = 0 end
    if cant_move then cant_move = 1 else cant_move = 0 end
    if always_on_top then always_on_top = 1 else always_on_top = 0 end

    local title_ptr = copy_string_to_memory(title)

    local window = profan.call_c(
        calls.get_lib_func(LIBDAUBE_ID, winfuncs.window_create),
        4, desktop_get_main(),
        4, title_ptr,
        4, x, 4, y, 4, width, 4, height,
        1, is_lite, 1, cant_move, 1, always_on_top
    )

    calls.free(title_ptr)

    return window
end

local function window_destroy(window)
    profan.call_c(calls.get_lib_func(LIBDAUBE_ID, winfuncs.window_delete), 4, window)
end

local function desktop_refresh()
    profan.call_c(calls.get_lib_func(LIBDAUBE_ID, winfuncs.desktop_refresh), 4, desktop_get_main())
end

local function window_set_pixel(window, x, y, color)
    profan.call_c(calls.get_lib_func(LIBDAUBE_ID, winfuncs.window_set_pixel), 4, window, 4, x, 4, y, 4, color, 4, 1)
end


local function window_refresh(window)
    profan.call_c(calls.get_lib_func(LIBDAUBE_ID, winfuncs.window_refresh), 4, window)
end

local function window_display_pixel(window, x, y, color)
    -- set pixel without refreshing
    profan.call_c(calls.get_lib_func(LIBDAUBE_ID, winfuncs.window_display_pixel), 4, window, 4, x, 4, y, 4, color, 4, 1)
end

return {
    window_create = window_create,
    window_destroy = window_destroy,

    window_set_pixel = window_set_pixel,
    window_display_pixel = window_display_pixel,
    window_refresh = window_refresh,

    desktop_refresh = desktop_refresh,
    desktop_get_main = desktop_get_main,
}
