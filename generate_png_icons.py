import struct
import zlib
import os

def create_png(width, height, r, g, b, filename):
    # Minimal pure python PNG generator without any 3rd party dependencies
    raw_data = bytearray()
    for y in range(height):
        raw_data.append(0)  # Filter type none
        for x in range(width):
            # Draw a nice rounded rectangle with inner circle
            dx = x - width / 2
            dy = y - height / 2
            dist = (dx*dx + dy*dy) ** 0.5
            
            # Rounded corner calculation
            corner_r = width * 0.22
            cx = abs(x - width / 2) - (width / 2 - corner_r)
            cy = abs(y - height / 2) - (height / 2 - corner_r)
            in_corner = cx > 0 and cy > 0
            corner_dist = (cx*cx + cy*cy) ** 0.5 if in_corner else 0
            
            if in_corner and corner_dist > corner_r:
                # Outside rounded icon
                raw_data.extend([0, 0, 0, 0])
            elif dist < width * 0.22:
                # White center circle
                raw_data.extend([255, 255, 255, 255])
            elif dist < width * 0.26 and abs(dx) < width * 0.05 and dy < 0:
                # Plug prongs
                raw_data.extend([79, 70, 229, 255])
            else:
                # Indigo background
                gradient_factor = y / height
                pr = int(r * (1 - 0.2 * gradient_factor))
                pg = int(g * (1 - 0.2 * gradient_factor))
                pb = int(b * (1 - 0.2 * gradient_factor))
                raw_data.extend([pr, pg, pb, 255])

    def chunk(tag, data):
        return struct.pack('>I', len(data)) + tag + data + struct.pack('>I', zlib.crc32(tag + data) & 0xffffffff)

    header = b'\x89PNG\r\n\x1a\n'
    ihdr = chunk(b'IHDR', struct.pack('>IIBBBBB', width, height, 8, 6, 0, 0, 0))
    idat = chunk(b'IDAT', zlib.compress(bytes(raw_data), 9))
    iend = chunk(b'IEND', b'')

    with open(filename, 'wb') as f:
        f.write(header + ihdr + idat + iend)

create_png(192, 192, 99, 102, 241, './icons/icon-192.png')
create_png(512, 512, 79, 70, 229, './icons/icon-512.png')
print("Successfully created icon-192.png and icon-512.png")
