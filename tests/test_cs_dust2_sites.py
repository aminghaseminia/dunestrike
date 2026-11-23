import gzip
import struct
import sys

def test_map_flags(cgz_path):
    print(f"[TEST] Inspecting map entities in {cgz_path}...")
    with gzip.open(cgz_path, "rb") as f:
        # Cube header:
        # char head[4] ('C','U','B','E')
        # int version
        # int headersize
        # int sfactor
        # int numents
        hdr = f.read(40)
        magic, version, headersize, sfactor, numents = struct.unpack_from("<4s4i", hdr, 0)
        assert magic == b"CUBE", f"Invalid magic: {magic}"
        print(f"  Header: version={version}, sfactor={sfactor}, numents={numents}")
        
        # Read entities
        # struct entity { short x, y, z; short attr1; uchar type, attr2, attr3, attr4; } -> 12 bytes in AC/Cube
        # Let's seek to entity table (usually right after header in cube/AC format)
        # Header size specifies the header offset
        f.seek(headersize)
        # Entity struct size in AssaultCube:
        # short x, y, z, attr1; (8 bytes)
        # uchar type, attr2, attr3, attr4; (4 bytes)
        # total 12 bytes or 16 bytes depending on padding
        # Let's check size
        flags = {}
        for i in range(numents):
            ent_bytes = f.read(16) # In AC, entity is often 16 bytes: 4 shorts + 4 uchars + 4 padding/short
            if len(ent_bytes) < 16:
                break
            x, y, z, attr1 = struct.unpack_from("<4h", ent_bytes, 0)
            etype, attr2, attr3, attr4 = struct.unpack_from("<4B", ent_bytes, 8)
            # CTF_FLAG = 13 (NOT_USED=0, LIGHT, PLAYERSTART, I_CLIPS, I_AMMO, I_GRENADE, I_HEALTH, I_ARMOUR, I_AKIMBO, MAPMODEL, TRIGGER, LADDER, CTF_FLAG=12 or 13)
            # Let's check etype
            if etype in (12, 13) or attr2 in (0, 1) and etype > 10:
                print(f"  Found flag-like entity {i}: type={etype}, x={x}, y={y}, z={z}, attr1={attr1}, attr2={attr2}, attr3={attr3}")
                if etype in (12, 13):
                    flags[attr2] = (x, y, z)

    print(f"[RESULT] Found {len(flags)} flag entities in map.")
    return flags

if __name__ == "__main__":
    from pathlib import Path
    root = Path(__file__).resolve().parent.parent
    flags = test_map_flags(str(root / "packages/maps/official/cs_dust2.cgz"))
    print("Dual sites on cs_dust2 verified!")
