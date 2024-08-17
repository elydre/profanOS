local calls = require("calls")

local function stop()
    profan.pout(0x61, 1, profan.pin(0x61, 1) & 0xFC)
end

local function freq(frequency)
    if frequency == 0 then
        stop()
        return
    end

    -- set the PIT to the desired frequency

    local div = 1193180 // frequency

    profan.pout(0x43, 1, 0xb6)
    profan.pout(0x42, 1, div & 0xff)
    profan.pout(0x42, 1, (div >> 8) & 0xff)

    -- and play the sound using the PC speaker
    local tmp = profan.pin(0x61, 1)
    if tmp ~= (tmp | 3) then
        profan.pout(0x61, 1, tmp | 3)
    end
end

-- Play() port from templeOS
-- Notes are entered with a capital letter.

-- Octaves are entered with a digit and
-- stay set until changed. Mid C is octave 4.

-- Durations are entered with
-- 'w' whole note
-- 'h' half note
-- 'q' quarter note
-- 'e' eighth note
-- 't' sets to 2/3rds the current duration
-- '.' sets to 1.5 times the current duration
-- durations stay set until changed.

-- Sharp and flat are done with '#' or 'b'.

local function get_frequency(note, octave)
    local base_frequency = 440          -- reference frequency for A4
    local octave_offset = (octave - 4)  -- number of octaves to offset
    local notes = {
      ["C"] = -9, ["C#"] = -8, ["Db"] = -8, ["D"] = -7,
      ["D#"] = -6, ["Eb"] = -6, ["E"] = -5, ["F"] = -4,
      ["F#"] = -3, ["Gb"] = -3, ["G"] = -2, ["G#"] = -1,
      ["Ab"] = -1, ["A"] = 0, ["A#"] = 1, ["Bb"] = 1, ["B"] = 2
    } -- table of notes and their offsets from A4

    return math.floor(base_frequency * (2 ^ (octave_offset + (notes[note] / 12))))
end

local function play_note(note, octave, duration)
    local frequency = get_frequency(note, octave)
    print(string.format("Playing %s%d for %d ms (freq: %d)", note, octave, duration, frequency))
    freq(frequency, 6)
    calls.ms_sleep(duration // 2)
    stop()
end

local function play(song)
    local current_octave = 4
    local current_duration = 1000

    local c
    local next_c

    for i = 1, #song do
        c = song:sub(i, i)

        if i < #song then
            next_c = song:sub(i + 1, i + 1)
        else
            next_c = ""
        end

        -- cheek if c is note
        if string.match(c, "[A-G]") then
            if next_c == "#" then
                c = c .. next_c
                i = i + 1
            elseif next_c == "b" then
                c = c .. next_c
                i = i + 1
            end
            play_note(c, current_octave, current_duration)
        elseif c == " " then
            -- Do nothing
        elseif c == "w" then
            current_duration = 4000
        elseif c == "h" then
            current_duration = 2000
        elseif c == "q" then
            current_duration = 1000
        elseif c == "e" then
            current_duration = 500
        elseif c == "t" then
            current_duration = math.floor(current_duration * 2 / 3)
        elseif c == "." then
            current_duration = math.floor(current_duration * 3 / 2)
        elseif c == "1" then
            current_octave = 1
        elseif c == "2" then
            current_octave = 2
        elseif c == "3" then
            current_octave = 3
        elseif c == "4" then
            current_octave = 4
        elseif c == "5" then
            current_octave = 5
        elseif c == "6" then
            current_octave = 6
        elseif c == "7" then
            current_octave = 7
        elseif c == "8" then
            current_octave = 8
        elseif c == "9" then
            current_octave = 9
        else
            print("Unknown character: " .. c)
        end
    end
end

return {
    freq = freq,
    stop = stop,
    play = play,
}
