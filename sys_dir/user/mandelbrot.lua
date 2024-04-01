-- Definition of image resolution
local width = 800
local height = 600

-- Definition of properties of the Mandelbrot set
local maxIterations = 256
local minReal = -2
local maxReal = 1
local minImag = -1
local maxImag = 1

-- Function to convert screen coordinates to complex coordinates
local function screenToComplex(x, y)
    local real = minReal + (maxReal - minReal) * (x / width)
    local imag = minImag + (maxImag - minImag) * (y / height)
    return real, imag
end

-- Function to check if a point is inside the Mandelbrot set
local function isInsideMandelbrot(real, imag, maxIterations)
    local zReal, zImag = 0, 0
    for i = 1, maxIterations do
        local zReal2, zImag2 = zReal * zReal, zImag * zImag
        zImag, zReal = 2 * zReal * zImag + imag, zReal2 - zImag2 + real
        if zReal2 + zImag2 > 4 then
            -- The point is outside the set
            return false, i
        end
    end
    -- The point is inside the set
    return true, maxIterations
end

-- Loop through all pixels of the image
for y = 0, height - 1 do
    for x = 0, width - 1 do
        local real, imag = screenToComplex(x, y)
        local isInside, iterations = isInsideMandelbrot(real, imag, maxIterations)
        -- Color of the pixel in shades of violet / blue
        local color = isInside and 0x000000 or 0x08000F * iterations
        profan.setpixel(x, y, color)
    end
end
