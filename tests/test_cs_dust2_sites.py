import gzip
import struct
import sys
from pathlib import Path

def test_map_flags(cgz_path):
    print(f"[TEST] Inspecting map entities in {cgz_path}...")
    with gzip.open(cgz_path, "rb") as f:
        hdr = f.read(40)
        magic, version, headersize, sfactor, numents = struct.unpack_from("<4s4i", hdr, 0)
        assert magic in (b"CUBE", b"ACMP"), f"Invalid magic: {magic}"
        print(f"  Header: magic={magic.decode('latin1', errors='ignore')}, version={version}, headersize={headersize}, sfactor={sfactor}, numents={numents}")
        
        f.seek(headersize)
        # In AssaultCube/Cube map formats:
        # If version < 10: 12 bytes per persistent entity:
        #   short x, y, z, attr1 (8 bytes) + uchar type, attr2, attr3, attr4 (4 bytes)
        # If version >= 10: 16 bytes per entity:
        #   short x, y, z, attr1 + uchar type, attr2, attr3, attr4 + 4 bytes reserved/short
        entsize = 12 if version < 10 else 16
        flags = {}
        spawns = []
        for i in range(numents):
            ent_bytes = f.read(entsize)
            if len(ent_bytes) < entsize:
                break
            x, y, z, attr1 = struct.unpack_from("<4h", ent_bytes, 0)
            etype, attr2, attr3, attr4 = struct.unpack_from("<4B", ent_bytes, 8)
            # CTF_FLAG = 12 in AssaultCube
            if etype == 12:
                flags[attr2] = (x, y, z)
                print(f"  [FLAG] Site {chr(ord('A') + attr2)} (ent {i}): pos=({x}, {y}, {z}), attr1={attr1}, attr2={attr2}, attr3={attr3}")
            elif etype == 2: # PLAYERSTART
                spawns.append((attr2, x, y, z))

    print(f"[RESULT] Found {len(flags)} bomb sites and {len(spawns)} player spawns.")
    assert 0 in flags, "Site A (flag 0) not found in cs_dust2!"
    assert 1 in flags, "Site B (flag 1) not found in cs_dust2!"
    
    # Verify expected competitive positions on cs_dust2
    assert flags[0] == (155, 41, 7), f"Site A pos mismatch: {flags[0]}"
    assert flags[1] == (138, 184, -11), f"Site B pos mismatch: {flags[1]}"
    print("  -> PASS: Dual bomb sites verified at Site A (155, 41, 7) and Site B (138, 184, -11)!")
    return flags

if __name__ == "__main__":
    root = Path(__file__).resolve().parent.parent
    cgz_file = root / "packages/maps/official/cs_dust2.cgz"
    flags = test_map_flags(str(cgz_file))
    print("Dual sites on cs_dust2 verified successfully!")
