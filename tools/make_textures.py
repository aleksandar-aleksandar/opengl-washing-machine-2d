"""
Generates the clothing textures (PNG files) for the washing machine project.

Run it from the project folder:   py tools/make_textures.py
It writes the images into the textures/ folder.

Every piece of clothing is drawn WHITE with light gray outlines on a transparent
background. The actual color (white, red, pink, blue...) is applied later in the
fragment shader by multiplying with uColor, so one image works for any color.

To add a new piece of clothing: add an entry to CLOTHES at the bottom.
Shapes use coordinates from 0 to 1:  (0, 0) = top left,  (1, 1) = bottom right.
"""

import math
import os
import struct
import zlib

SIZE = 256            # image width and height in pixels
OUTLINE = 6.0         # outline thickness in pixels
LINE = 4.0            # thickness of detail lines (seams, collar...) in pixels

FILL_GRAY = 255       # brightness of the inside   (255 = white)
LINE_GRAY = 175       # brightness of the outlines (darker = more visible)


# ---------------------------------------------------------------- geometry

def dist_to_segment(px, py, ax, ay, bx, by):
    """Distance from point P to the line segment A-B."""
    abx, aby = bx - ax, by - ay
    t = ((px - ax) * abx + (py - ay) * aby) / (abx * abx + aby * aby)
    t = max(0.0, min(1.0, t))
    cx, cy = ax + t * abx, ay + t * aby
    return math.hypot(px - cx, py - cy)


def inside_polygon(px, py, pts):
    """Even-odd rule: shoot a ray to the right, count how many edges it crosses."""
    inside = False
    n = len(pts)
    for i in range(n):
        ax, ay = pts[i]
        bx, by = pts[(i + 1) % n]
        if (ay > py) != (by > py):
            x_cross = ax + (py - ay) * (bx - ax) / (by - ay)
            if px < x_cross:
                inside = not inside
    return inside


# ---------------------------------------------------------------- drawing

def draw(outline, details):
    """Returns RGBA pixel rows for one piece of clothing."""
    # convert 0..1 coordinates to pixels
    poly = [(x * SIZE, y * SIZE) for x, y in outline]
    segs = [((ax * SIZE, ay * SIZE), (bx * SIZE, by * SIZE)) for (ax, ay), (bx, by) in details]
    edges = [(poly[i], poly[(i + 1) % len(poly)]) for i in range(len(poly))]

    rows = []
    for y in range(SIZE):
        row = bytearray()
        for x in range(SIZE):
            px, py = x + 0.5, y + 0.5   # center of the pixel

            # distance to the shape's border (positive inside, negative outside)
            d_edge = min(dist_to_segment(px, py, *a, *b) for a, b in edges)
            sd = d_edge if inside_polygon(px, py, poly) else -d_edge

            # alpha: 1 inside, 0 outside, smooth over 1 pixel at the border (anti-aliasing)
            alpha = max(0.0, min(1.0, sd + 0.5))

            # how much "line" color this pixel gets: near the border, or near a detail line
            line = max(0.0, min(1.0, OUTLINE - sd + 0.5))
            for (a, b) in segs:
                d = dist_to_segment(px, py, *a, *b)
                line = max(line, max(0.0, min(1.0, LINE / 2 - d + 0.5)))

            gray = FILL_GRAY + (LINE_GRAY - FILL_GRAY) * line
            g = int(round(gray))
            row += bytes((g, g, g, int(round(alpha * 255))))
        rows.append(bytes(row))
    return rows


def draw_colored(size, layers, outline_px):
    """Like draw(), but in COLOR, for images that aren't tinted later (the cursor).
    layers = list of (points, fill_rgb, outline_rgb), drawn in order, later ones on top."""
    rows = []
    for y in range(size):
        row = bytearray()
        for x in range(size):
            px, py = x + 0.5, y + 0.5
            r = g = b = a = 0.0                  # start fully transparent
            for points, fill, line_rgb in layers:
                poly = [(u * size, v * size) for u, v in points]
                edges = [(poly[i], poly[(i + 1) % len(poly)]) for i in range(len(poly))]
                d_edge = min(dist_to_segment(px, py, *e[0], *e[1]) for e in edges)
                sd = d_edge if inside_polygon(px, py, poly) else -d_edge
                cover = max(0.0, min(1.0, sd + 0.5))
                if cover == 0.0:
                    continue
                line = max(0.0, min(1.0, outline_px - sd + 0.5))
                cr = fill[0] + (line_rgb[0] - fill[0]) * line
                cg = fill[1] + (line_rgb[1] - fill[1]) * line
                cb = fill[2] + (line_rgb[2] - fill[2]) * line
                # paint this layer over what's already there ("over" blending, like glBlendFunc)
                r = cr * cover + r * (1 - cover)
                g = cg * cover + g * (1 - cover)
                b = cb * cover + b * (1 - cover)
                a = cover + a * (1 - cover)
            if a > 0:                            # r, g, b were mixed with "nothing" at soft edges; undo that
                r, g, b = r / a, g / a, b / a
            row += bytes((int(round(r)), int(round(g)), int(round(b)), int(round(a * 255))))
        rows.append(bytes(row))
    return rows


def circle(cx, cy, radius, steps=24):
    """A circle as a polygon with many corners."""
    return [(cx + radius * math.cos(2 * math.pi * i / steps),
             cy + radius * math.sin(2 * math.pi * i / steps)) for i in range(steps)]


def tilt(points, degrees, scale=1.0):
    """Shrink by 'scale' and rotate by 'degrees' around the image center (negative = counter-clockwise)."""
    t = math.radians(degrees)
    out = []
    for x, y in points:
        dx, dy = (x - 0.5) * scale, (y - 0.5) * scale
        out.append((0.5 + dx * math.cos(t) - dy * math.sin(t),
                    0.5 + dx * math.sin(t) + dy * math.cos(t)))
    return out


# ---------------------------------------------------------------- PNG file writing

def write_png(path, rows):
    def chunk(kind, data):
        c = struct.pack(">I", len(data)) + kind + data
        return c + struct.pack(">I", zlib.crc32(kind + data) & 0xFFFFFFFF)

    size = len(rows)
    raw = b"".join(b"\x00" + r for r in rows)   # each row starts with filter type 0
    png = b"\x89PNG\r\n\x1a\n"
    png += chunk(b"IHDR", struct.pack(">IIBBBBB", size, size, 8, 6, 0, 0, 0))  # 8-bit RGBA
    png += chunk(b"IDAT", zlib.compress(raw, 9))
    png += chunk(b"IEND", b"")
    with open(path, "wb") as f:
        f.write(png)


# ---------------------------------------------------------------- the clothes
# Each entry: "name": (outline points, detail lines)
#   outline points = the shape's border, in order around the shape
#   detail lines   = extra lines drawn on top: ((x1, y1), (x2, y2))

CLOTHES = {
    "tshirt": (
        [(0.38, 0.12), (0.50, 0.20), (0.62, 0.12),            # neck
         (0.80, 0.17), (0.97, 0.38), (0.85, 0.48), (0.76, 0.41),  # right sleeve
         (0.76, 0.90), (0.24, 0.90),                          # bottom
         (0.24, 0.41), (0.15, 0.48), (0.03, 0.38), (0.20, 0.17)],  # left sleeve
        [((0.41, 0.135), (0.50, 0.235)), ((0.50, 0.235), (0.59, 0.135)),  # collar
         ((0.24, 0.41), (0.21, 0.19)), ((0.76, 0.41), (0.79, 0.19))],     # sleeve seams
    ),
    "pants": (
        [(0.28, 0.08), (0.72, 0.08), (0.80, 0.94), (0.56, 0.94),
         (0.50, 0.38), (0.44, 0.94), (0.20, 0.94)],
        [((0.275, 0.17), (0.725, 0.17)),   # waistband
         ((0.50, 0.17), (0.50, 0.32))],    # zipper
    ),
    "sock": (
        [(0.30, 0.06), (0.60, 0.06), (0.60, 0.58), (0.82, 0.67),
         (0.91, 0.78), (0.84, 0.91), (0.42, 0.92), (0.30, 0.82)],
        [((0.30, 0.17), (0.60, 0.17)),     # cuff
         ((0.70, 0.64), (0.68, 0.91))],    # toe
    ),
    "dress": (
        [(0.36, 0.06), (0.42, 0.06), (0.45, 0.22), (0.55, 0.22), (0.58, 0.06),
         (0.64, 0.06), (0.66, 0.30), (0.62, 0.45), (0.85, 0.94),
         (0.15, 0.94), (0.38, 0.45), (0.34, 0.30)],
        [((0.38, 0.45), (0.62, 0.45))],    # waist
    ),
}


# ---------------------------------------------------------------- the cursor: a fabric softener bottle
# Designed standing upright, then tilted so the cap points to the top left (like pouring).
# The cursor's "click point" (hotspot) is the tip of the cap.

CURSOR_SIZE = 64
CURSOR_TILT = -40      # degrees, negative = counter-clockwise
CURSOR_SCALE = 0.85

BOTTLE_BLUE = (120, 190, 255)
DARK_BLUE = (35, 80, 150)
CAP_BLUE = (60, 110, 210)
WHITE = (255, 255, 255)
PINK = (255, 130, 180)

BOTTLE_LAYERS = [
    # cap
    ([(0.40, 0.04), (0.60, 0.04), (0.60, 0.19), (0.40, 0.19)], CAP_BLUE, DARK_BLUE),
    # bottle body (neck, shoulders, body)
    ([(0.42, 0.18), (0.58, 0.18), (0.62, 0.27), (0.76, 0.33), (0.78, 0.94),
      (0.22, 0.94), (0.24, 0.33), (0.38, 0.27)], BOTTLE_BLUE, DARK_BLUE),
    # label
    ([(0.31, 0.48), (0.69, 0.48), (0.69, 0.80), (0.31, 0.80)], WHITE, DARK_BLUE),
    # a little pink flower dot on the label
    (circle(0.50, 0.64, 0.09), PINK, PINK),
]
CURSOR_HOTSPOT = (0.50, 0.04)   # top of the cap, before tilting


if __name__ == "__main__":
    out_dir = os.path.join(os.path.dirname(__file__), "..", "textures")
    os.makedirs(out_dir, exist_ok=True)
    for name, (outline, details) in CLOTHES.items():
        path = os.path.join(out_dir, name + ".png")
        write_png(path, draw(outline, details))
        print("wrote", os.path.normpath(path))

    # the arrow button icon: a "<" chevron (the ">" is the same image drawn mirrored in C++)
    arrow = [(0.62, 0.12), (0.76, 0.26), (0.52, 0.50), (0.76, 0.74), (0.62, 0.88), (0.24, 0.50)]
    path = os.path.join(out_dir, "arrow.png")
    write_png(path, draw(arrow, []))
    print("wrote", os.path.normpath(path))

    # the cursor (outline is thinner because the image is small)
    layers = [(tilt(pts, CURSOR_TILT, CURSOR_SCALE), fill, line) for pts, fill, line in BOTTLE_LAYERS]
    path = os.path.join(out_dir, "cursor_bottle.png")
    write_png(path, draw_colored(CURSOR_SIZE, layers, outline_px=2.0))
    hx, hy = tilt([CURSOR_HOTSPOT], CURSOR_TILT, CURSOR_SCALE)[0]
    print("wrote", os.path.normpath(path),
          "- hotspot (put in main.cpp):", round(hx * CURSOR_SIZE), round(hy * CURSOR_SIZE))
