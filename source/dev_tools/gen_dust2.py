#!/usr/bin/env python3
"""
Generate a de_dust2-inspired map for Dune Strike.

The layout follows the classic CS2 radar/callout map: T spawn at the south,
T ramp and outside tunnel on the west, B tunnels and B site in the north-west,
mid with mid doors in the centre, CT spawn at the north, short/stairs and the
catwalk between mid and A, A site and long A in the north-east, plus the pit
and long doors.

The engine stores maps as a heightfield of cubical columns: every cell is
either SOLID or a SPACE column with a floor and a ceiling height.  A 1-cube
height difference between neighbouring columns is climbable by the player
(the physics allows a step up of about 1.26 cubes), so height changes are
built as 1-cube steps.

This tool writes the .cgz container directly: header, entities and the
run-length encoded cube array.  Run it from the repository root:

    python3 source/dev_tools/gen_dust2.py [--preview out.png]

Texture slots come from config/default_map_settings.cfg; the map's texture
tables are the identity mapping, so a cube index is a texture slot.
"""

import gzip
import os
import struct
import sys
import time

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
OUT_MAP = os.path.join(ROOT, "packages/maps/official/de_dust2.cgz")

HEADER_FMT = "<4siiii128s768si4Biiii40s"
ENTITY_FMT = "<hhhhBBBBhbb"

SFACTOR = 7
SSIZE = 1 << SFACTOR

# cube types (world.h)
SOLID, CORNER, FHF, CHF, SPACE = 0, 1, 2, 3, 4

# texture slots from config/default_map_settings.cfg
WALL = 25          # makke/d_brick - warm brown brick
FLOOR = 141        # craig_fortune/terrain/sand_bleached
FLOOR_IN = 152     # noctua/ground/ground02 - packed dirt for tunnels
SKY = 0            # hardcoded DEFAULT_SKY index
CEIL = 25          # brick ceiling
UPPER = 25

AMBIENT = 45
WATERLEVEL = -100000

# ---------------------------------------------------------------------------
# walkable areas, coordinates in the 128x128 grid, y grows to the north
# (x1, y1, x2, y2 inclusive, floor, ceiling, indoor)
# ---------------------------------------------------------------------------
AREAS = [
    # --- T side ---
    ("t-spawn",          30,   4,  82,  19,   0, 18, False),
    ("t-ramp",            2,   4,  34,  26,   0, 18, False),
    ("outside-tunnel",    2,  24,  32,  49,   0,  9, True),
    ("b-tunnel",          2,  47,  26,  76,   0,  8, True),
    ("b-approach",       10,  74,  28,  94,   0,  9, True),
    ("b-site",           10,  94,  36, 120,   1, 18, False),
    ("b-doors-lower",    26,  90,  48, 100,   0, 12, True),
    ("suicide",          54,  16,  60,  38,   0, 18, False),
    ("taxi",             77,  18,  88,  38,   0, 18, False),
    # --- mid ---
    ("top-mid",          50,  36,  79,  48,   0, 18, False),
    ("mid",              55,  44,  65,  82,   0, 18, False),
    ("mid-doors",        53,  80,  63,  92,   0, 12, True),
    ("lower-mid",        45,  90,  79, 100,   0, 18, False),
    ("ct-boost",         63,  88,  73,  99,   0, 18, False),
    ("ct-spawn",         77,  94,  92, 116,   1, 18, False),
    ("catwalk",          62,  62,  70,  76,   0, 12, True),
    ("x-box",            54,  70,  61,  78,   0, 18, False),
    ("ct-mid",           66,  68,  80,  78,   0, 18, False),
    ("stairs",           78,  76,  86,  86,   0, 12, True),
    ("short",            78,  80,  86,  98,   1, 12, True),
    ("short-boost",      85,  94, 100, 102,   1, 18, False),
    # --- A side ---
    ("a-site",           98, 100, 118, 118,   1, 18, False),
    ("a-ramp",          114, 100, 125, 112,   1, 18, False),
    ("a-long",          110,  64, 125, 102,   0, 18, False),
    ("pit",             112,  46, 124,  64,   0, 18, False),
    ("blue-box",         86,  62, 112,  72,   0, 18, False),
    ("long-doors",       88,  42,  96,  64,   0, 12, True),
    ("long-corner",      76,  38,  96,  50,   0, 18, False),
]

# solid fill applied after carving, to add cover and break sight lines
WALLS = [
    # B site building and crates
    (16, 100, 22, 108),
    (28, 110, 30, 112),
    # A site building
    (102, 104, 108, 110),
    (110, 114, 112, 116),
    # mid boxes
    (56, 58, 58, 60), (62, 52, 64, 54),
    # long cover
    (116, 74, 120, 78),
    # CT spawn block
    (82, 100, 86, 104),
    # T spawn block
    (36,  8,  40,  11),
    # B tunnel pillar
    (12,  58,  14,  60),
    # catwalk stub
    (68,  70,  70,  72),
    # B doors choke
    (10, 90, 14, 94), (24, 90, 28, 94),
    # mid doors choke
    (53, 84, 55, 86), (61, 84, 63, 86),
    # long doors choke
    (88, 50, 90, 54), (94, 50, 96, 54),
    # short/stairs corner
    (84, 76, 86, 78),
]

# ---------------------------------------------------------------------------
# entities
# ---------------------------------------------------------------------------
LIGHT, PLAYERSTART, I_CLIPS, I_AMMO, I_GRENADE, I_HEALTH = 1, 2, 3, 4, 5, 6
MAPMODEL, CTF_FLAG = 10, 13


def entity(x, y, z, type, a1=0, a2=0, a3=0, a4=0, a5=0, a6=0, a7=0):
    return (x, y, z, a1, type, a2, a3, a4, a5, a6, a7)


def build_entities():
    ents = []
    # team spawns: CLA (team 0) at T spawn, RVSF (team 1) at CT spawn.
    # yaw 180 faces north, yaw 0 faces south (see physics.cpp movement vector).
    for x in (34, 42, 50, 58, 66, 74):
        for y in (6, 8, 10):
            ents.append(entity(x, y, 0, PLAYERSTART, 0, 0))
    for x in (80, 85, 90):
        for y in (110, 112, 114):
            ents.append(entity(x, y, 1, PLAYERSTART, 180, 1))
    # bomb sites, stored as CTF flag entities
    ents.append(entity(30, 106, 1, CTF_FLAG, 0, 0))   # B
    ents.append(entity(112, 108, 1, CTF_FLAG, 0, 1))  # A
    # exactly two ammo and two health pickups
    ents.append(entity(60, 58, 0, I_AMMO))
    ents.append(entity(118, 80, 0, I_AMMO))
    ents.append(entity(30, 116, 1, I_HEALTH))
    ents.append(entity(114, 104, 1, I_HEALTH))
    # indoor lights
    for x, y, z in [
        (16, 30, 6), (10, 40, 6), (14, 52, 6), (18, 64, 6), (18, 78, 6),
        (20, 88, 6), (56, 84, 6), (58, 88, 6), (64, 66, 6), (66, 72, 6),
        (82, 78, 6), (82, 90, 6), (90, 48, 6), (92, 58, 6),
    ]:
        ents.append(entity(x, y, z, LIGHT, 12, 220))
    # outdoor fill lights
    for x, y, z in [
        (34, 108, 12), (114, 110, 12), (60, 40, 14), (60, 94, 14),
        (112, 80, 12), (82, 24, 10), (56, 24, 10), (84, 108, 10),
        (118, 52, 12), (24, 64, 10), (94, 66, 12), (48, 94, 12),
    ]:
        ents.append(entity(x, y, z, LIGHT, 18, 180))
    # crates and barrels for cover (mapmodel slots from default_map_settings)
    cover = [
        (100, 102, 1, 131, 0), (110, 108, 1, 131, 0), (104, 114, 1, 37, 0),
        (112, 104, 1, 34, 0), (100, 116, 1, 34, 0),
        (26, 102, 1, 131, 0), (32, 116, 1, 131, 0), (12, 112, 1, 37, 0),
        (34, 100, 1, 34, 0), (14, 100, 1, 34, 0),
        (60, 62, 0, 131, 0), (116, 70, 0, 131, 0), (58, 46, 0, 112, 0),
        (112, 88, 0, 112, 0), (82, 20, 0, 37, 0), (30, 22, 0, 34, 0),
        (66, 74, 0, 112, 0), (92, 66, 0, 34, 0),
    ]
    for x, y, z, slot, angle in cover:
        ents.append(entity(x, y, z, MAPMODEL, angle, slot, 0, 0))
    return ents


# ---------------------------------------------------------------------------
# map writer
# ---------------------------------------------------------------------------
def texlists():
    return bytes(range(256)) * 3


def rle(cubes):
    out = bytearray()
    i, n = 0, len(cubes)
    while i < n:
        c = cubes[i]
        j = i + 1
        while j < n and cubes[j] == c:
            j += 1
        emit(out, c)
        run = j - i - 1
        while run > 0:
            chunk = min(run, 255)
            out += bytes((255, chunk))
            run -= chunk
        i = j
    return bytes(out)


def emit(out, c):
    t, floor, ceil, wtex, ftex, ctex, vdelta, utex, tag = c
    if t == SOLID:
        out += bytes((SOLID, wtex, vdelta))
    else:
        out += bytes((t, floor & 0xff, ceil & 0xff, wtex, ftex, ctex,
                      vdelta, utex, tag))


def build_world():
    solid = (SOLID, 0, 0, WALL, 0, 0, 0, 0, 0)
    cubes = [solid] * (SSIZE * SSIZE)

    def fill(x1, y1, x2, y2, cube):
        for y in range(max(0, y1), min(y2, SSIZE - 1) + 1):
            row = y * SSIZE
            for x in range(max(0, x1), min(x2, SSIZE - 1) + 1):
                cubes[row + x] = cube

    for name, x1, y1, x2, y2, floor, ceil, indoor in AREAS:
        fill(x1, y1, x2, y2,
             (SPACE, floor, ceil, WALL, FLOOR_IN if indoor else FLOOR,
              CEIL if indoor else SKY, 0, UPPER, 0))
    for x1, y1, x2, y2 in WALLS:
        fill(x1, y1, x2, y2, solid)
    return cubes


def check():
    world = build_world()
    ents = build_entities()
    bad = []
    for e in ents:
        x, y, z, a1, t, *_ = e
        c = world[y * SSIZE + x]
        if c[0] == SOLID:
            bad.append((x, y, z, t, "inside solid"))
        elif t == PLAYERSTART and c[1] != z:
            bad.append((x, y, z, t, "floor %d" % c[1]))
    labels = {LIGHT: "light", PLAYERSTART: "spawn", MAPMODEL: "model",
              CTF_FLAG: "site", I_AMMO: "ammo", I_HEALTH: "health"}
    for b in bad:
        print("bad entity %s at %d,%d,%d: %s" % (labels.get(b[3], b[3]), b[0], b[1], b[2], b[4]))
    return not bad


def preview(path):
    from PIL import Image, ImageDraw
    img = Image.new("RGB", (SSIZE * 4, SSIZE * 4), (20, 20, 30))
    d = ImageDraw.Draw(img)
    scale = 4
    for y in range(SSIZE):
        for x in range(SSIZE):
            c = WORLD_CACHE[y * SSIZE + x]
            px = x * scale
            py = (SSIZE - 1 - y) * scale
            if c[0] == SOLID:
                col = (45, 45, 55)
            elif c[1] == 0:
                col = (150, 120, 70) if c[5] == SKY else (110, 90, 60)
            else:
                col = (90, 140, 90) if c[5] == SKY else (60, 100, 60)
            d.rectangle([px, py, px + scale - 1, py + scale - 1], fill=col)
    for e in build_entities():
        x, y, z, a1, t, a2, *_ = e
        px = x * scale
        py = (SSIZE - 1 - y) * scale
        col = {PLAYERSTART: (0, 160, 255), CTF_FLAG: (255, 60, 60),
               I_AMMO: (255, 255, 0), I_HEALTH: (0, 255, 120),
               MAPMODEL: (200, 200, 200), LIGHT: (255, 220, 120)}.get(t, (255, 0, 255))
        d.rectangle([px - 2, py - 2, px + 2, py + 2], fill=col)
    img.save(path)
    print("wrote preview", path)


WORLD_CACHE = None


def main():
    global WORLD_CACHE
    WORLD_CACHE = build_world()
    if "--preview" in sys.argv:
        preview(sys.argv[sys.argv.index("--preview") + 1])
        return
    if not check():
        print("entity check failed")
        sys.exit(1)
    ents = build_entities()
    title = b"de_dust2 - Dune Strike edition"[:127]
    header = struct.pack(
        HEADER_FMT,
        b"ACMP", 10, struct.calcsize(HEADER_FMT), SFACTOR, len(ents),
        title.ljust(128, b"\0"), texlists(), WATERLEVEL, 0, 0, 0, 0,
        1, AMBIENT, 0, int(time.time()), b"\0" * 40,
    )
    body = bytearray(header)
    for e in ents:
        body += struct.pack(ENTITY_FMT, *e)
    body += rle(WORLD_CACHE)
    os.makedirs(os.path.dirname(OUT_MAP), exist_ok=True)
    with gzip.open(OUT_MAP, "wb", compresslevel=9) as f:
        f.write(bytes(body))
    print("wrote %s: %d bytes, %d entities, %dx%d cubes" %
          (OUT_MAP, os.path.getsize(OUT_MAP), len(ents), SSIZE, SSIZE))


if __name__ == "__main__":
    main()
