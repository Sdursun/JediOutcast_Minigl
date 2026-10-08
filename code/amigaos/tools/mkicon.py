#!/usr/bin/env python3
"""Writes jk2sp.info: a WBTOOL icon with a classic 4-colour image (for
Workbench 1.3-3.1) and an OS 3.5 GlowIcon (FORM ICON with FACE/IMAG).
The picture is drawn here (a blue lightsaber), no game artwork is used.

usage: mkicon.py <output.info> [stacksize]
"""
import math
import struct
import sys

W, H = 46, 46

# ---- the picture: palette indexes, 0 = transparent ------------------------
PAL = [
    (0, 0, 0),        # 0 transparent
    (10, 10, 20),     # 1 outline
    (90, 90, 100),    # 2 hilt dark
    (170, 170, 180),  # 3 hilt mid
    (235, 235, 240),  # 4 hilt light / blade core
    (200, 30, 30),    # 5 button
    (20, 40, 140),    # 6 glow outer
    (40, 90, 220),    # 7 glow mid
    (110, 170, 255),  # 8 glow inner
    (200, 230, 255),  # 9 blade
]

pix = [[0] * W for _ in range(H)]


def put(x, y, c, over=True):
    if 0 <= x < W and 0 <= y < H and (over or pix[y][x] == 0):
        pix[y][x] = c


# saber axis from the hilt (bottom left) to the tip (top right)
x0, y0 = 5.0, 42.0
x1, y1 = 41.0, 4.0
hilt_len = 0.27          # fraction of the axis that is hilt
dx, dy = x1 - x0, y1 - y0
L = math.hypot(dx, dy)
ux, uy = dx / L, dy / L

for y in range(H):
    for x in range(W):
        px, py = x + 0.5 - x0, y + 0.5 - y0
        t = (px * ux + py * uy) / L          # position along the axis
        d = abs(px * uy - py * ux)           # distance from the axis
        if t < -0.02 or t > 1.02:
            continue
        if t <= hilt_len:
            if d <= 2.6:
                band = int(t * 40) % 5 == 0
                if d > 1.9:
                    put(x, y, 1)
                elif band:
                    put(x, y, 2)
                else:
                    put(x, y, 4 if d < 0.6 else 3)
        else:
            tip = min(1.0, (1.0 - t) * 12)   # rounded tip
            if d <= 0.8 * tip + 0.2:
                put(x, y, 4)
            elif d <= 1.6 * tip + 0.2:
                put(x, y, 9)
            elif d <= 2.6 * tip:
                put(x, y, 8)
            elif d <= 3.7 * tip:
                put(x, y, 7)
            elif d <= 4.8 * tip:
                put(x, y, 6)

# emitter ring and activation button
for y in range(H):
    for x in range(W):
        px, py = x + 0.5 - x0, y + 0.5 - y0
        t = (px * ux + py * uy) / L
        d = abs(px * uy - py * ux)
        if abs(t - hilt_len) < 0.025 and d <= 3.2:
            put(x, y, 1 if d > 2.4 else 3)
        if abs(t - 0.14) < 0.03 and 0.4 < (px * uy - py * ux) < 2.0:
            put(x, y, 5)

# ---- classic 4-colour image (Workbench pens: 0 grey, 1 black, 2 white, 3 blue)
CLASSIC = {0: 0, 1: 1, 2: 1, 3: 2, 4: 2, 5: 1, 6: 3, 7: 3, 8: 3, 9: 2}


def planar(width, height, depth, getpix):
    rowbytes = ((width + 15) // 16) * 2
    out = bytearray()
    for plane in range(depth):
        for y in range(height):
            row = bytearray(rowbytes)
            for x in range(width):
                if getpix(x, y) & (1 << plane):
                    row[x >> 3] |= 0x80 >> (x & 7)
            out += row
    return bytes(out)


def packbits(data):
    """ByteRun1, as the IMAG chunk's RLE uses with 8-bit elements."""
    out = bytearray()
    i, n = 0, len(data)
    while i < n:
        run = 1
        while i + run < n and run < 128 and data[i + run] == data[i]:
            run += 1
        if run >= 2:
            out += struct.pack('b', -(run - 1)) + bytes([data[i]])
            i += run
            continue
        j = i
        while j < n and j - i < 128 and not (j + 1 < n and data[j] == data[j + 1]):
            j += 1
        out += struct.pack('b', j - i - 1) + data[i:j]
        i = j
    return bytes(out)


def chunk(cid, body):
    if len(body) & 1:
        body += b'\0'
        return cid + struct.pack('>I', len(body) - 1) + body
    return cid + struct.pack('>I', len(body)) + body


def glowicon():
    face = struct.pack('>BBBBH', W - 1, H - 1, 0, 0x11, len(PAL) * 3 - 1)
    img = bytes(pix[y][x] for y in range(H) for x in range(W))
    rle = packbits(img)
    pal = b''.join(bytes(c) for c in PAL)
    imag = struct.pack('>BBBBBBHH', 0, len(PAL) - 1, 0x03, 1, 0, 8,
                       len(rle) - 1, len(pal) - 1) + rle + pal
    form = b'ICON' + chunk(b'FACE', face) + chunk(b'IMAG', imag)
    return b'FORM' + struct.pack('>I', len(form)) + form


def diskobject(stack):
    gadget = struct.pack('>IhhhhHHHIIIIIHI',
                         0, 0, 0, W, H + 1,
                         0x0004,        # GFLG_GADGIMAGE, complement highlight
                         0x0003,        # RELVERIFY | GADGIMMEDIATE
                         0x0001,        # BOOLGADGET
                         1, 0, 0, 0, 0, 0,
                         1)             # UserData: WB_DISKREVISION
    do = struct.pack('>HH', 0xE310, 1) + gadget + struct.pack(
        '>BBIIiiIII', 3, 0,            # WBTOOL
        0, 0,                          # no default tool, no tool types
        -0x80000000, -0x80000000,      # NO_ICON_POSITION
        0, 0, stack)
    assert len(do) == 78, len(do)
    image = struct.pack('>hhhhhIBBI', 0, 0, W, H, 2, 1, 3, 0, 0)
    data = planar(W, H, 2, lambda x, y: CLASSIC[pix[y][x]])
    return do + image + data


def main():
    out = sys.argv[1]
    stack = int(sys.argv[2]) if len(sys.argv) > 2 else 200000
    with open(out, 'wb') as f:
        f.write(diskobject(stack) + glowicon())
    for y in range(H):
        print(''.join(' .:-=#*%@&'[pix[y][x]] for x in range(W)))


if __name__ == '__main__':
    main()
