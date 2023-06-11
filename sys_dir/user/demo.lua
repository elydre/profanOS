calls = require("calls")
win = require("win")

-- create a window and refresh desktop
wptr = win.window_create(100, 100, "Hello world!")
win.desktop_refresh()

-- sleep for 2 seconds
calls.ms_sleep(2000)

d = profan.ticks()

-- draw a rectangle (with refresh)
for i = 0, 99 do
    for j = 0, 99 do
        win.window_display_pixel(wptr, i, j, 0xFF0000)
    end
end

e = profan.ticks()

print("Time: " .. (e - d) .. "ms")


-- sleep for 2 seconds
calls.ms_sleep(2000)

-- destroy window
win.window_destroy(wptr)
