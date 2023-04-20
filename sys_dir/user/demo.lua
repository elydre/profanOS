-- Définition de la résolution de l'image
local width = 800
local height = 600

-- Définition des propriétés de l'ensemble de Mandelbrot
local maxIterations = 256
local minReal = -2
local maxReal = 1
local minImag = -1
local maxImag = 1

-- Fonction pour convertir les coordonnées d'écran en coordonnées complexes
local function screenToComplex(x, y)
    local real = minReal + (maxReal - minReal) * (x / width)
    local imag = minImag + (maxImag - minImag) * (y / height)
    return real, imag
end

-- Fonction pour vérifier si un point est à l'intérieur de l'ensemble de Mandelbrot
local function isInsideMandelbrot(real, imag, maxIterations)
    local zReal, zImag = 0, 0
    for i = 1, maxIterations do
        local zReal2, zImag2 = zReal * zReal, zImag * zImag
        zImag, zReal = 2 * zReal * zImag + imag, zReal2 - zImag2 + real
        if zReal2 + zImag2 > 4 then
            return false, i
        end
    end
    return true, maxIterations
end

-- Parcours de tous les pixels de l'image
for y = 0, height - 1 do
    for x = 0, width - 1 do
        local real, imag = screenToComplex(x, y)
        local isInside, iterations = isInsideMandelbrot(real, imag, maxIterations)
        local color = isInside and 0x000000 or 0xFFFFFF
        profan.setpixel(x, y, color)
    end
end