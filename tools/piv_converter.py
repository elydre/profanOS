from PIL import Image
import os

"""
file format:
| type | width | height | frame_nb | pixels |
| u8   | u16   | u16    | u16      | ...    |
"""


path = os.path.dirname(os.path.abspath(__file__))

IMAGE = 0xFF
VIDEO = 0xFE

def convert_image_to_piv(image):
    im = Image.open(image)
    pixels = im.load()

    width, height = im.size

    output = [IMAGE, width & 0xFF, width >> 8 & 0xFF, height & 0xFF, height >> 8 & 0xFF, 0, 0]

    for y in range(height):
        for x in range(width):
            r, g, b, a = pixels[x, y]
            output.extend((r, g, b))

    with open(f'{path}/test.piv', 'wb') as f:
        f.write(bytearray(output))

def convert_gif_to_piv(gif):
    im = Image.open(gif)
    frame_nb = im.n_frames - 1

    width, height = im.size

    output = [VIDEO, width & 0xFF, width >> 8 & 0xFF, height & 0xFF, height >> 8 & 0xFF, frame_nb & 0xFF, frame_nb >> 8 & 0xFF]

    for frame in range(1, frame_nb + 1):
        im.seek(frame)
        for y in range(height):
            for x in range(width):
                rgb = im.getpixel((x, y))
                if isinstance(rgb, int):
                    r = rgb
                    g = rgb
                    b = rgb
                else:
                    r, g, b, a = rgb
                output.extend((r, g, b))
    
    with open(f'{path}/test.piv', 'wb') as f:
        f.write(bytearray(output))

convert_gif_to_piv(f'{path}/rickroll.gif')
