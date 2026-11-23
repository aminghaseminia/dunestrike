#!/usr/bin/env python3
"""
Generate a de_dust2-inspired map for Dune Strike.

The engine stores maps as a heightfield of cubical columns: every cell is
either SOLID or a SPACE column with a floor and a ceiling height.  A 1-cube
height difference between neighbouring columns is climbable by the player
(the physics allows a step up of about 1.26 cubes), so ramps are built as
short stair cases of 1-cube steps.

The map is described as a handful of carved rooms/corridors plus solid walls
that cut the big rooms into lanes, and a set of entities (spawns, two bomb
sites stored as CTF flags, two ammo and two health pickups, lights and a few
crates).

This tool writes the .cgz container directly: header, entities and the
run-length encoded cube array.  Run it from the repository root:

    python3 source/dev_tools/gen_dust2.py

Texture slots come from config/default_map_settings.cfg; the map's texture
tables are the identity mapping, so a cube index is a texture slot.
"""

import gzip
import os
import struct
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
WALL = 26          # 3dcafe/stone18 - grey/brown stone wall
FLOOR = 141        # craig_fortune/terrain/sand_bleached
FLOOR_IN = 152     # noctua/ground/ground02 - packed dirt for tunnels
SKY = 0            # hardcoded DEFAULT_SKY index
CEIL = 26          # stone ceiling
UPPER = 26

AMBIENT = 45
WATERLEVEL = -100000

# ---------------------------------------------------------------------------
# blueprint: carved rooms and corridors (x1, y1, x2, y2 inclusive)
# ---------------------------------------------------------------------------
AREAS = [
    # name              x1  y1   x2   y2  floor ceil indoor
    ("t-spawn",          88, 10, 114,  28,    0,  18, False),
    ("t-corridor",       44, 16,  92,  24,    0,  18, False),
    ("b-tunnel-south",   38, 12,  46,  27,    0,   8, True),
    ("b-tunnel-north",   32, 26,  40,  56,    0,   8, True),
    ("b-tunnel-upper",   28, 56,  40,  78,    1,   8, True),
    ("b-site",           14, 72,  38,  94,    1,  18, False),
    ("ct-b",             18, 94,  36, 108,    1,  10, True),
    ("ct-spawn",         34,106,  74, 120,    1,  18, False),
    ("ct-a",             74, 98,  90, 112,    1,  10, True),
    ("a-site",           84, 74, 112,  98,    1,  18, False),
    ("long-lower",      104, 32, 112,  68,    0,  18, False),
    ("long-upper",      104, 68, 112,  76,    1,  18, False),
    ("long-conn",       100, 26, 112,  34,    0,  18, False),
    ("mid-lower",        60, 30,  68,  64,    0,  18, False),
    ("mid-upper",        60, 64,  68, 106,    1,  18, False),
    ("mid-t",            60, 22,  68,  32,    0,  18, False),
    ("t-mid",            66, 18,  90,  26,    0,  18, False),
    ("lower-tunnel",     38, 66,  62,  72,    1,  10, True),
    ("catwalk",          68, 82,  86,  90,    1,  12, True),
]

# solid fill applied after carving, to break the big rooms into lanes
WALLS = [
    # A site: central building and two wall stubs (leaving a gap between them)
    (92, 80, 100, 88),
    (84, 92, 89, 94), (95, 92, 101, 94),
    # B site: building and wall stubs
    (18, 76, 25, 83),
    (14, 88, 20, 90), (25, 88, 31, 90),
    # mid doors
    (60, 50, 62, 53), (66, 50, 68, 53),
    # long doors
    (104, 48, 106, 51), (110, 48, 112, 51),
    # b tunnel pillar
    (34, 39, 36, 42),
    # catwalk side stubs
    (70, 82, 72, 84), (80, 88, 82, 90),
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
    # team spawns: CLA (team 0) in the south-east, RVSF (team 1) in the north
    for x in (92, 99, 106, 112):
        for y in (12, 18, 24):
            ents.append(entity(x, y, 0, PLAYERSTART, 1800, 0))
    for x in (38, 46, 54, 62, 70):
        for y in (108, 114):
            ents.append(entity(x, y, 1, PLAYERSTART, 2700, 1))
    # the two bomb sites (bomb mode reads CTF flag entities as sites)
    ents.append(entity(32, 86, 1, CTF_FLAG, 0, 0))   # B
    ents.append(entity(88, 78, 1, CTF_FLAG, 0, 1))   # A
    # exactly two ammo and two health pickups
    ents.append(entity(64, 40, 0, I_AMMO))
    ents.append(entity(108, 40, 0, I_AMMO))
    ents.append(entity(24, 92, 1, I_HEALTH))
    ents.append(entity(104, 94, 1, I_HEALTH))
    # indoor lights
    for x, y, z in [
        (42, 18, 6), (40, 34, 6), (36, 48, 6), (34, 60, 7), (34, 72, 7),
        (24, 100, 7), (32, 104, 7), (80, 104, 6), (86, 108, 6),
        (44, 68, 7), (52, 68, 7), (58, 70, 7), (74, 84, 7), (78, 86, 7),
    ]:
        ents.append(entity(x, y, z, LIGHT, 12, 220))
    # outdoor fill lights over the open areas
    for x, y, z in [
        (28, 82, 12), (88, 80, 12), (62, 40, 14), (62, 92, 14),
        (108, 58, 12), (104, 30, 12), (46, 20, 10), (56, 112, 10),
        (100, 20, 10), (30, 108, 10), (108, 72, 12), (30, 80, 10),
    ]:
        ents.append(entity(x, y, z, LIGHT, 18, 180))
    # crates and barrels for cover (mapmodel slot indices from default_map_settings)
    cover = [
        (96, 76, 1, 131, 0), (102, 84, 1, 131, 0), (94, 94, 1, 37, 0),
        (86, 90, 1, 34, 0), (106, 88, 1, 34, 0),
        (22, 74, 1, 131, 0), (34, 90, 1, 131, 0), (16, 86, 1, 37, 0),
        (34, 79, 1, 34, 0), (20, 86, 1, 34, 0),
        (64, 56, 0, 131, 0), (104, 58, 0, 131, 0), (64, 34, 0, 112, 0),
        (106, 44, 0, 112, 0), (106, 20, 0, 37, 0), (44, 16, 0, 34, 0),
        (62, 82, 1, 112, 0), (108, 68, 0, 34, 0),
    ]
    for x, y, z, slot, angle in cover:
        ents.append(entity(x, y, z, MAPMODEL, angle, slot, 0, 0))
    return ents


# ---------------------------------------------------------------------------
# map writer
# ---------------------------------------------------------------------------
def texlists():
    # identity mapping: cube texture index i -> global texture slot i
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


def main():
    tl = texlists()
    cubes = build_world()
    ents = build_entities()
    title = b"de_dust2 - Dune Strike edition"[:127]
    header = struct.pack(
        HEADER_FMT,
        b"ACMP", 10, struct.calcsize(HEADER_FMT), SFACTOR, len(ents),
        title.ljust(128, b"\0"), tl, WATERLEVEL, 0, 0, 0, 0,
        1, AMBIENT, 0, int(time.time()), b"\0" * 40,
    )
    body = bytearray(header)
    for e in ents:
        body += struct.pack(ENTITY_FMT, *e)
    body += rle(cubes)
    os.makedirs(os.path.dirname(OUT_MAP), exist_ok=True)
    with gzip.open(OUT_MAP, "wb", compresslevel=9) as f:
        f.write(bytes(body))
    print("wrote %s: %d bytes, %d entities, %dx%d cubes" %
          (OUT_MAP, os.path.getsize(OUT_MAP), len(ents), SSIZE, SSIZE))


if __name__ == "__main__":
    main()
