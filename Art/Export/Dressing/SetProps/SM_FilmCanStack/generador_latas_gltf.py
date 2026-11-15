"""Procedural low-poly stack of 1940s 35mm film cans -> single .glb (meters, Y-up, pivot at base center).
Flat-shaded (faceted) primitives with flat PBR colors, matching D-015 taste (muted greys/olive, no texture)."""
import json, math, struct, sys

SEG = 14  # facets per can: reads faceted, like the reference
MATS = {  # name: (rgb, roughness)
    "CanGrey": ((0.36, 0.38, 0.40), 0.85),
    "CanOlive": ((0.24, 0.25, 0.20), 0.9),
    "LidLight": ((0.46, 0.48, 0.50), 0.85),
    "Film": ((0.05, 0.05, 0.05), 0.6),
    "Tape": ((0.72, 0.64, 0.50), 0.95),
}
tris = {k: [] for k in MATS}  # material -> list of (p0,p1,p2)


def rot_y(p, a):
    c, s = math.cos(a), math.sin(a)
    return (p[0] * c + p[2] * s, p[1], -p[0] * s + p[2] * c)


def rot_x(p, a):
    c, s = math.cos(a), math.sin(a)
    return (p[0], p[1] * c - p[2] * s, p[1] * s + p[2] * c)


def xf(p, off, ay=0.0, ax=0.0, pivot=(0, 0, 0)):
    p = (p[0] - pivot[0], p[1] - pivot[1], p[2] - pivot[2])
    p = rot_x(p, ax)
    p = rot_y(p, ay)
    return (p[0] + pivot[0] + off[0], p[1] + pivot[1] + off[1], p[2] + pivot[2] + off[2])


def cylinder(mat, r, y0, y1, off=(0, 0, 0), ay=0.0, ax=0.0, top=True, bottom=True, pivot=(0, 0, 0)):
    ring = [(r * math.cos(2 * math.pi * i / SEG), r * math.sin(2 * math.pi * i / SEG)) for i in range(SEG)]
    T = lambda x, y, z: xf((x, y, z), off, ay, ax, pivot)
    for i in range(SEG):
        a, b = ring[i], ring[(i + 1) % SEG]
        p0, p1 = T(a[0], y0, a[1]), T(b[0], y0, b[1])
        p2, p3 = T(b[0], y1, b[1]), T(a[0], y1, a[1])
        tris[mat] += [(p0, p2, p1), (p0, p3, p2)]
        if top:
            tris[mat].append((T(0, y1, 0), T(b[0], y1, b[1]), T(a[0], y1, a[1])))
        if bottom:
            tris[mat].append((T(0, y0, 0), T(a[0], y0, a[1]), T(b[0], y0, b[1])))


def box(mat, cx, cy, cz, sx, sy, sz, off=(0, 0, 0), ay=0.0):
    v = [(cx + dx * sx / 2, cy + dy * sy / 2, cz + dz * sz / 2) for dx in (-1, 1) for dy in (-1, 1) for dz in (-1, 1)]
    v = [xf(p, off, ay) for p in v]
    faces = [(0, 1, 3, 2), (4, 6, 7, 5), (0, 4, 5, 1), (2, 3, 7, 6), (0, 2, 6, 4), (1, 5, 7, 3)]
    for a, b, c, d in faces:
        tris[mat] += [(v[a], v[b], v[c]), (v[a], v[c], v[d])]


R, H, LID = 0.18, 0.072, 0.022  # 36 cm diameter cans, 7.2 cm tall, lid band 2.2 cm
stack = [  # (x, z offset, yaw, body mat)
    (0.000, 0.000, 0.0, "CanOlive"),
    (0.025, -0.015, 0.4, "CanOlive"),
    (-0.030, 0.010, 0.9, "CanGrey"),
    (0.030, 0.020, 1.4, "CanOlive"),
]
y = 0.0
for i, (x, z, ay, mat) in enumerate(stack):
    off = (x, y, z)
    cylinder(mat, R, 0.0, H - LID, off, ay, top=False)
    cylinder(mat if mat == "CanOlive" else "LidLight", R + 0.006, H - LID, H, off, ay)
    if i in (1, 2):  # blank paper tape labels on the side
        box("Tape", R + 0.004, H * 0.5, 0.0, 0.006, 0.03, 0.11, off, ay + 0.3 + i)
    y += H
# top can: open, film reel visible, lid resting ajar
off = (-0.01, y, 0.0)
cylinder("CanGrey", R, 0.0, H - 0.01, off, 0.2, top=False)
cylinder("Film", R - 0.025, 0.004, H - 0.024, off, 0.2)
cylinder("CanGrey", 0.03, 0.004, H - 0.02, off, 0.2)  # reel hub
lid_off = (0.03, y + H - 0.004, -0.02)
cylinder("LidLight", R + 0.006, 0.0, LID, lid_off, 0.6, ax=0.16, pivot=(0, 0, R))

# --- write glb (flat normals: unshared verts per triangle) ---
bin_parts, buffer_views, accessors, prims, materials = [], [], [], [], []
offset = 0


def add_view(data, target):
    global offset
    pad = (-len(data)) % 4
    bin_parts.append(data + b"\0" * pad)
    buffer_views.append({"buffer": 0, "byteOffset": offset, "byteLength": len(data), "target": target})
    offset += len(data) + pad
    return len(buffer_views) - 1


for mi, (name, (rgb, rough)) in enumerate(MATS.items()):
    materials.append({"name": name, "pbrMetallicRoughness": {"baseColorFactor": [*rgb, 1.0], "metallicFactor": 0.0, "roughnessFactor": rough}})
    pos, nor = [], []
    for p0, p1, p2 in tris[name]:
        u = [p1[k] - p0[k] for k in range(3)]
        v = [p2[k] - p0[k] for k in range(3)]
        n = (u[1] * v[2] - u[2] * v[1], u[2] * v[0] - u[0] * v[2], u[0] * v[1] - u[1] * v[0])
        L = math.sqrt(sum(c * c for c in n)) or 1.0
        n = tuple(c / L for c in n)
        for p in (p0, p1, p2):
            pos.append(p)
            nor.append(n)
    if not pos:
        continue
    pb = b"".join(struct.pack("<3f", *p) for p in pos)
    nb = b"".join(struct.pack("<3f", *n) for n in nor)
    mn = [min(p[k] for p in pos) for k in range(3)]
    mx = [max(p[k] for p in pos) for k in range(3)]
    accessors.append({"bufferView": add_view(pb, 34962), "componentType": 5126, "count": len(pos), "type": "VEC3", "min": mn, "max": mx})
    accessors.append({"bufferView": add_view(nb, 34962), "componentType": 5126, "count": len(nor), "type": "VEC3"})
    prims.append({"attributes": {"POSITION": len(accessors) - 2, "NORMAL": len(accessors) - 1}, "material": mi})

binblob = b"".join(bin_parts)
gltf = {
    "asset": {"version": "2.0", "generator": "tdc latas_gltf.py"},
    "scene": 0, "scenes": [{"nodes": [0]}],
    "nodes": [{"name": "SM_FilmCanStack", "mesh": 0}],
    "meshes": [{"name": "SM_FilmCanStack", "primitives": prims}],
    "materials": materials, "accessors": accessors, "bufferViews": buffer_views,
    "buffers": [{"byteLength": len(binblob)}],
}
js = json.dumps(gltf, separators=(",", ":")).encode()
js += b" " * ((-len(js)) % 4)
glb = struct.pack("<III", 0x46546C67, 2, 12 + 8 + len(js) + 8 + len(binblob))
glb += struct.pack("<II", len(js), 0x4E4F534A) + js + struct.pack("<II", len(binblob), 0x004E4942) + binblob
open(sys.argv[1], "wb").write(glb)
ntri = sum(len(t) for t in tris.values())
allp = [p for t in tris.values() for tri in t for p in tri]
print("tris", ntri, "height_m", round(max(p[1] for p in allp), 3), "min_y", round(min(p[1] for p in allp), 4))
