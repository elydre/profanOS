sound = require("sound")
calls = require("calls")

i = 0x100000

while (profan.pin(0x60, 1) ~= 1) do
    mem = profan.memval(i, 1)
    print(string.format("0x%07x: %02x", i, mem))

    if (mem ~= 0) then
        sound.freq(mem * 5 + 20)
    else 
        sound.stop()
    end
    
    calls.ms_sleep(20)
    i = i + 1
end


sound.stop()
