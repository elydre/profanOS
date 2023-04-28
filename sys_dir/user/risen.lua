-- Play the templeOS theme song 'Risen' by Terry A. Davis
-- Source:  Sup1Hymns/TOSTheme.HC.Z
--          Adam/ASnd.HC

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


sound = require("sound")
-- sound.play_sound(freq)
-- sound.nosound()

calls = require("calls")
-- calls.ms_sleep(ms)

function get_frequency(note, octave)
    local base_frequency = 440 -- Fréquence de référence A4
    local octave_offset = (octave - 4) -- A4 est l'octave 4, donc on calcule l'offset pour la note demandée
    local notes = {
      ["C"] = -9, ["C#"] = -8, ["Db"] = -8, ["D"] = -7,
      ["D#"] = -6, ["Eb"] = -6, ["E"] = -5, ["F"] = -4,
      ["F#"] = -3, ["Gb"] = -3, ["G"] = -2, ["G#"] = -1,
      ["Ab"] = -1, ["A"] = 0, ["A#"] = 1, ["Bb"] = 1, ["B"] = 2
    } -- Tableau pour stocker les notes et leurs offsets de fréquence

    local note_frequency = math.floor(base_frequency * (2 ^ (octave_offset + (notes[note] / 12))))
    return note_frequency
end

function play_sound(note, octave, duration)
    local freq = get_frequency(note, octave)
    print(string.format("Playing %s%d for %d ms (freq: %d)", note, octave, duration, freq))
    sound.play_sound(freq, 6)
    calls.ms_sleep(duration // 2)
    sound.nosound()
end

function play(song)
    local current_octave = 4
    local current_duration = 1000

    local c;
    local next_c;

    for i = 1, #song do
        c = song:sub(i, i)

        if i < #song then
            next_c = song:sub(i + 1, i + 1)
        else
            next_c = ""
        end

        -- cheek if c is note
        if c == "C" or c == "D" or c == "E" or c == "F" or c == "G" or c == "A" or c == "B" then
            if next_c == "#" then
                c = c .. next_c
                i = i + 1
            elseif next_c == "b" then
                c = c .. next_c
                i = i + 1
            end
            play_sound(c, current_octave, current_duration)
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

print("-- For my hero, Terry Davis --")

play("5eDEqFFetEEFqDeCDDEetCGF")
play("5eDEqFFetEEFqDeCDDEetCGF")
play("5eDCqDE4eAA5etEEFEDG4B5DCqF")
play("5eDCqDE4eAA5etEEFEDG4B5DCqF")
