local calls = require("calls")

function play_sound(freq)
    -- set the PIT to the desired frequency

    local div = 1193180 // freq

    profan.pout(0x43, 1, 0xb6)
    profan.pout(0x42, 1, div & 0xff)
    profan.pout(0x42, 1, (div >> 8) & 0xff)

    -- and play the sound using the PC speaker
    local tmp = profan.pin(0x61, 1)
    if tmp ~= (tmp | 3) then
        profan.pout(0x61, 1, tmp | 3)
    end
end


function nosound()
    profan.pout(0x61, 1, profan.pin(0x61, 1) & 0xFC)
end


-- demo function
function beep()
    -- play 1KHz sound
    play_sound(1000)

    -- wait for 1 sec
    calls.ms_sleep(1000)

    -- stop the sound
    nosound()
end


return {
    play_sound = play_sound,
    nosound = nosound,
    beep = beep,
}
