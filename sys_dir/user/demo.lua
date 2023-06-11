calls = require("calls")
win = require("win")

-- create a window and refresh desktop
wptr = win.window_create(100, 100, "Hello world!")
win.desktop_refresh()


-- draw a rectangle (with refresh)
for i = 10, 20 do
    for j = 10, 20 do
        win.window_set_pixel(wptr, i, j, 0xFF0000)
    end
end

win.window_refresh(wptr)


-- draw a rectangle (without refresh)
for i = 30, 40 do
    for j = 10, 20 do
        win.window_display_pixel(wptr, i, j, 0x00FF00)
    end
end

-- sleep for 2 seconds
calls.ms_sleep(2000)

-- destroy window
win.window_destroy(wptr)
