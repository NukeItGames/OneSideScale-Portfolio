"""Create the original portfolio icon using only Python's standard library."""
from pathlib import Path
import struct
import zlib


def icon_bytes():
    size = 64
    pixels = bytearray(size * size * 4)

    def rect(x0, y0, x1, y1, color):
        for y in range(y0, y1):
            for x in range(x0, x1):
                i = (y * size + x) * 4
                pixels[i:i + 4] = bytes(color)

    # A fixed left edge, rectangular bounds, and a rightward face handle.
    white, blue = (225, 233, 243, 255), (46, 183, 245, 255)
    rect(8, 14, 12, 50, white)
    rect(12, 14, 40, 18, white)
    rect(12, 46, 40, 50, white)
    rect(36, 18, 40, 46, blue)
    rect(40, 30, 57, 34, blue)
    for step in range(8):
        rect(49 + step, 24 + step, 50 + step, 40 - step, blue)

    def chunk(kind, data):
        return struct.pack('!I', len(data)) + kind + data + struct.pack('!I', zlib.crc32(kind + data))

    raw = b''.join(b'\x00' + pixels[y * size * 4:(y + 1) * size * 4] for y in range(size))
    return (b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('!IIBBBBB', size, size, 8, 6, 0, 0, 0))
            + chunk(b'IDAT', zlib.compress(raw, 9)) + chunk(b'IEND', b''))


if __name__ == '__main__':
    destination = Path(__file__).resolve().parents[1] / 'Resources/icon.png'
    destination.write_bytes(icon_bytes())
    print(destination)
