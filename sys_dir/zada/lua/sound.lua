function play_sound(div)
    -- Set the PIT to the desired frequency

    profan.pout(0x43, 1, 0xb6)
    profan.pout(0x42, 1, div & 0xff)
    profan.pout(0x42, 1, (div >> 8) & 0xff)

    -- And play the sound using the PC speaker
    tmp = profan.pin(0x61, 1)
    if tmp ~= (tmp | 3) then
        profan.pout(0x61, 1, tmp | 3)
    end
end


function nosound()
    profan.pout(0x61, 1, profan.pin(0x61, 1) & 0xFC)
end

-- Make a beep
function beep(time)
    play_sound(1193) -- 1193180 // freq
    
    start = profan.ticks()
    while profan.ticks() - start < time do
        -- do nothing
    end
    
    nosound()
    -- set_PIT_2(old_frequency)
end

    
return {
    play_sound = play_sound,
    nosound = nosound,
    beep = beep,
}
